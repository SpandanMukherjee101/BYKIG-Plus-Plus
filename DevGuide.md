# BYKIG++ Developer Guide

This guide explains the current implementation architecture and is intended for contributors or anyone extending the interpreter.

## 1. Repository structure

```text
BYKIG++/
├── Interpreter.c
├── VarLL.h
├── ArithmaticExprEval.h
├── ExprEvalStack.h
├── Builtins.h
├── Keywords.h
└── BYKIG.c
```

### Responsibilities

| File | Responsibility |
|---|---|
| `Interpreter.c` | Main token-driven interpreter loop, declarations, statements, control flow, functions, modules, and program entry. |
| `VarLL.h` | Runtime structures, variable/collection storage, lookup, mutation, and frame cleanup. |
| `ArithmaticExprEval.h` | Expression evaluation and complex-expression resolution. |
| `ExprEvalStack.h` | Operator stack and `Value` stack implementation. |
| `Builtins.h` | Built-in handlers and built-in registry. |
| `Keywords.h` | Keyword-to-mode classification. |
| `BYKIG.c` | Project source/build-side support inherited from the project. |

## 2. Runtime architecture

The interpreter has five major pieces of mutable global state:

```text
callStack
    Active function invocations

envStack
    Active variable environments

FN
    Function-definition registry

current_fp
    Currently executing source file

global_fp
    Original top-level source file
```

The main entry point opens the source and invokes:

```c
run_interpreter_loop(fp, filepath);
```

## 3. Token-driven execution

`run_interpreter_loop()` repeatedly reads the next token with `fscanf()`, classifies it using `whichone()`, and executes a `switch` case.

There is no traditional compiler pipeline:

```text
Source
  ↓
fscanf()
  ↓
whichone()
  ↓
switch(mode)
  ↓
runtime operation
```

The current interpreter does not build an AST or a conventional parse tree.

## 4. `Value` model

`VarLL.h` defines:

```c
enum ValueType {
    VAL_FLOAT,
    VAL_STRING,
    VAL_INT_ARRAY,
    VAL_FLOAT_ARRAY,
    VAL_CHAR_ARRAY,
    VAL_BOOL_ARRAY,
    VAL_LIST,
    VAL_MAP,
    VAL_ERROR
};
```

and:

```c
struct Value {
    enum ValueType type;
    float f;
    char s[1000];
    void *ptr;
};
```

Use the fields according to `type`:

```text
VAL_FLOAT      → f
VAL_STRING     → s
array/list/map → ptr
VAL_ERROR      → error state
```

The `Value` object is the runtime transport format. Storage itself remains specialized.

## 5. Environment model

An environment is represented by the storage pointers inside `callFrame`:

```text
IV/FV/CV/BV/SV
IA/FA/CA/BA
LV/MV
```

`envStack[envTop]` is the active environment.

Lookup walks from the current environment toward global scope:

```text
for (idx = envTop; idx >= 0; idx--)
```

This makes local declarations shadow outer declarations.

## 6. Memory ownership and cleanup

`cleanupFrame()` is responsible for releasing frame-owned allocations.

It frees:

```text
primitive variable nodes
string variable nodes
array nodes + data buffers
list nodes + data buffers
map nodes
```

Function-call cleanup now occurs before restoring the caller's environment:

```text
callee environment
      ↓
cleanupFrame()
      ↓
envTop = saved_envTop
```

This is important for recursion because every recursive invocation gets a separate environment.

## 7. Function runtime

A function is represented by:

```c
struct funcDef {
    char name[100];
    int paramCount;
    struct funcParam params[10];
    long bodyPos;
    char filepath[1000];
    struct funcDef *next;
};
```

A definition records metadata rather than storing a body AST.

At call time:

```text
findFunc()
   ↓
evaluate arguments
   ↓
push call frame
   ↓
create environment
   ↓
materialize parameters
   ↓
fseek(bodyPos)
   ↓
run_interpreter_loop()
```

## 8. Call stack and recursion

The call stack is:

```c
struct callFrame callStack[10000];
int callStackTop = -1;
```

Each invocation stores return state and the caller's environment depth.

Recursion works by repeatedly creating:

```text
new call frame
new environment
```

for the same `funcDef`.

The current complex-expression resolver contains a call-depth guard at about 1000 frames (`callStackTop >= 999`), even though the static array is sized for 10000 entries. Treat the implementation guard as authoritative when changing recursion behavior.

## 9. Expression architecture

Expressions are handled in two stages:

```text
expression
   ↓
resolve_complex_types()
   ↓
functions / built-ins / indexing resolved
   ↓
val()
   ↓
postfix evaluation
   ↓
Value
```

`val()` uses:

```text
operator stack → struct node
value stack    → struct node2 containing Value
```

This avoids needing an AST for ordinary expressions.

## 10. Complex expression resolution

`resolve_complex_types()` recognizes constructs such as:

```text
function(args)
builtin(args)
array[index]
list[index]
map[key]
```

It evaluates the complex part and rewrites the token array so ordinary expression evaluation can continue.

This means a call such as:

```text
foo(2 + 3) * 4
```

can become conceptually:

```text
5 * 4
```

before ordinary postfix evaluation.

## 11. Operator precedence

`priority()` defines precedence for the internal operator representation.

Important internal equality operators are represented with single characters so the expression evaluator can use a character stack:

```text
~  equality
!  inequality
< > <= >=
& |
```

The expression implementation rewrites/handles multi-character comparisons before stack evaluation.

## 12. Built-in architecture

`Builtins.h` defines:

```c
typedef struct Value (*BuiltinHandler)(char args[][1000], int argCount);
```

and:

```c
struct BuiltinFunc {
    char name[100];
    int expectedArgs;
    BuiltinHandler handler;
};
```

The registry is:

```text
builtins[]
    ↓
findBuiltin()
    ↓
handler
```

To add a built-in:

1. Implement a handler returning `struct Value`.
2. Add an entry to `builtins[]`.
3. Ensure argument parsing and expression resolution can invoke it correctly.

## 13. Storage model for collections

### Arrays

Each primitive array type has its own node type and data pointer:

```text
iA → int *
fA → float *
cA → char *
bA → int *
```

### Lists

Lists use:

```text
size
capacity
float *data
```

### Maps

Maps use fixed storage:

```text
keys[100][100]
values[100]
count
```

and perform linear search.

## 14. Modules

`use` opens another `.bykig` file and invokes the same:

```c
run_interpreter_loop(mod_fp, resolved_path);
```

The interpreter temporarily changes:

```text
current_fp
```

to the module and restores it afterward.

Imported code runs in the current environment rather than a separate module namespace.

Function definitions created while importing retain the module path in `funcDef.filepath`, allowing later calls to reopen the defining file and seek to `bodyPos`.

## 15. Source-position control

The interpreter relies heavily on:

```c
ftell()
fseek()
```

for:

```text
while-loop restart positions
conditional skipping
function body entry
module switching
caller-position restoration
```

This is the main substitute for an explicit instruction pointer/AST execution tree.

## 16. Error handling

`safe_fscanf()` turns required-token read failures into a fatal error instead of allowing execution to continue with missing input.

Other runtime checks include:

```text
undefined names
invalid function/builtin lookup
stack overflow protection
break/continue outside a loop
collection access failures
file open/read/write failures
```

When extending the interpreter, preserve explicit failure behavior rather than silently manufacturing a valid `Value`.

## 17. Adding a new data type

A new runtime type generally requires changes across several layers:

```text
ValueType
    ↓
Value representation
    ↓
storage structure
    ↓
append/get/set functions
    ↓
typeFetcher()
    ↓
valFetcherGlobal()
    ↓
expression resolution
    ↓
declaration/assignment syntax
    ↓
cleanupFrame()
    ↓
function-argument copying
    ↓
built-ins / len / delete / exists where relevant
```

The important rule is to update ownership and cleanup at the same time as storage.

## 18. Adding a new statement

For a new keyword:

```text
Keywords.h
   ↓
new mode
   ↓
Interpreter.c switch case
   ↓
parse statement
   ↓
update runtime state
   ↓
handle errors
```

If the feature affects expressions, also integrate it with `resolve_complex_types()` and/or `val()`.

## 19. Practical development checklist

Before considering an interpreter feature complete, test:

```text
normal execution
nested execution
function-local variables
shadowing
nested function calls
recursion
error paths
module interaction
repeated execution
allocation/deallocation
```

For memory-sensitive changes, use an external memory checker such as AddressSanitizer or Valgrind in addition to language-level tests.

## 20. Architectural principles

When extending BYKIG++, preserve these project principles unless intentionally redesigning the runtime:

1. Keep storage ownership explicit.
2. Return runtime results through `Value` where possible.
3. Keep complex-expression handling separate from ordinary operator evaluation.
4. Preserve source-position restoration across nested execution.
5. Free frame-owned allocations when a scope lifetime ends.
6. Test nested and recursive cases, not only flat examples.
