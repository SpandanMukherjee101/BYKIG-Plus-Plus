# BYKIG++ Developer Guide

This guide explains the current implementation architecture and is intended for contributors or anyone extending the interpreter.

## 1. Repository structure

```text
BYKIG++/
├── Interpreter.c            # Main interpreter loop, statements, declarations, CLI
├── VarLL.h                  # Runtime structures, Value, environments, collection storage
├── ArithmaticExprEval.h     # Infix expression evaluator, complex resolution, postfix stack
├── ExprEvalStack.h          # Operator stack and Value stack implementation
├── Builtins.h               # Built-in function handlers and registry
├── Keywords.h               # Keyword-to-mode classification
├── BYKIG.c                  # Core support routines
├── tests/                   # Regression test suites and automated test runner
│   ├── run_tests.py         # Test execution harness (79 tests)
│   ├── basics/              # Core types, expressions, shadowing, strings
│   ├── control_flow/        # Conditionals, while loops, for loops, break/continue
│   ├── data_structures/     # Arrays, dynamic lists, generic maps, collection references
│   ├── functions/           # Function calls, recursion, scope, return values
│   ├── modules/             # File imports, circular dependency tests
│   └── integration/         # Sorting, mock database, prime sieve, file analyzers
└── mini-bykig++/            # Self-hosted companion interpreter written in BYKIG++
    ├── main.bykig           # CLI loop, statement dispatch, block execution
    ├── memory.bykig         # Lexical environments, heap lists/maps, function registry
    ├── evaluator.bykig      # Expressions, collection literals, nested indexing
    └── test_*.mini          # Test suites for mini-bykig++
```

### Responsibilities

| File | Responsibility |
|---|---|
| `Interpreter.c` | Main token-driven interpreter loop, declarations, statements, control flow (`if`, `while`, `for`, `break`, `continue`), functions, modules, and program entry. |
| `VarLL.h` | Unified `Value` model, variable/collection storage (`IV`, `SV`, `LV`, `MV`), lookup, mutation, reference aliasing, and frame cleanup. |
| `ArithmaticExprEval.h` | Multi-pass expression evaluator: complex resolution (`resolve_complex_types`), nested collection indexing, string token formatting (`format_string_token`), and postfix operator stack evaluation. |
| `ExprEvalStack.h` | Operator stack (`struct node`) and `Value` stack (`struct node2`) implementation. |
| `Builtins.h` | Built-in handlers, streaming file scanner (`file_scan_*`) with punctuation-aware tokenization, collection built-ins, and utility functions. |
| `Keywords.h` | Keyword-to-mode classification (`whichone`). |
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
VAL_LIST       → ptr points to struct lV
VAL_MAP        → ptr points to struct mV
array types    → ptr points to typed array struct
VAL_ERROR      → error state
```

The `Value` object is the runtime transport format. String conversion is handled via `value_to_string(struct Value v, char *dest, size_t dest_sz)`.

## 5. Environment model

An environment is represented by the storage pointers inside `callFrame`:

```text
IV/FV/CV/BV/SV   (primitive variables)
IA/FA/CA/BA      (typed arrays)
LV/MV            (dynamic lists and generic maps)
```

`envStack[envTop]` is the active environment.

Lookup walks from the current environment toward global scope:

```c
for (int idx = envTop; idx >= 0; idx--)
```

This makes local declarations shadow outer declarations. Assignments to existing variables search from the innermost scope outwards and mutate the existing binding rather than creating a duplicate.

## 6. Memory ownership and cleanup

`cleanupFrame()` is responsible for releasing frame-owned allocations.

It frees:

```text
primitive variable nodes
string variable nodes
array nodes + data buffers
list nodes + data buffers (for non-reference lists)
map nodes + entry buffers
```

Function-call cleanup occurs before restoring the caller's environment:

```text
callee environment
      ↓
cleanupFrame()
      ↓
envTop = saved_envTop
```

This guarantees that every recursive invocation gets a clean environment and avoids memory leaks upon unwind.

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
materialize parameters (copy values or pass list reference)
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

The complex-expression resolver contains a recursion-depth guard at about 1000 frames (`callStackTop >= 999`), protecting the process against runaway stack exhaustion.

## 9. Expression architecture

Expressions are handled in two stages:

```text
expression
   ↓
resolve_complex_types()
   ↓
functions / built-ins / multi-level collection indexing resolved
   ↓
format_string_token()
   ↓
val()
   ↓
postfix evaluation
   ↓
Value
```

`val()` uses:

```text
operator stack → struct node (ExprEvalStack.h)
value stack    → struct node2 containing struct Value
```

This avoids needing an AST for expressions while supporting arbitrary nesting.

## 10. Complex expression resolution and string tokens

`resolve_complex_types()` recognizes and resolves:

```text
function(args)
builtin(args)
array[index]
list[index]...[index]
map[key]...[key]
```

### Multi-level nested indexing
When a collection identifier is followed by `[`, `resolve_complex_types()` loops across all consecutive bracket pairs:

```c
while (nextIdx < tokenCount && strcmp(tokens[nextIdx], "[") == 0) { ... }
```

It recursively evaluates each index expression via `val()`, traverses down nested lists (`struct lV`) or maps (`struct mV`), and substitutes the final resolved element.

### String token formatting & unescaping
To prevent string contents (such as spaces, quotes, or punctuation) from breaking the postfix tokenizer:
- `format_string_token()` wraps string values in outer quotes and escapes inner `"` to `\"` and `\` to `\\`.
- The postfix evaluator strips the outer quotes and unescapes `\"` into `"` and `\\` into `\`.

This ensures string concatenation (`greeting + " " + target + "!"`) works cleanly without losing whitespace or confusing string operators.

## 11. Operator precedence

`priority()` defines precedence for internal operator representation:

```text
~, #   equality (numeric or strcmp for strings)
!, @   inequality
<, >   less/greater than
^, ?   less-or-equal, greater-or-equal
+, -   addition / subtraction (and string concatenation)
*, /, % multiplication, division, modulo
&, |   logical AND, logical OR
```

The expression tokenizer rewrites multi-character operators (`==`, `!=`, `<=`, `>=`, `&&`, `||`) to single-character internal tokens prior to stack evaluation.

## 12. Built-in architecture

`Builtins.h` defines:

```c
typedef struct Value (*BuiltinHandler)(char args[][1000], int argCount);

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

### Core Built-in Groups
- **Collection**: `len`, `push`, `pop`, `exists`, `delete`.
- **String utilities**: `quote`, `strip_quotes`, `is_num`, `is_str`, `starts_with`, `parse_float`.
- **File I/O**: `file_exists`, `file_write`, `file_append`, `file_read`.
- **Streaming scanner**: `file_scan_open`, `file_scan_next`, `file_scan_is_str`, `file_scan_close`, `file_scan_get_pos`, `file_scan_set_pos`.

### Punctuation-aware tokenizer in `file_scan_next`
`builtin_file_scan_next()` treats `[`, `]`, `{`, `}`, `(`, `)`, `,`, and `;` as self-delimiting tokens, allowing natural code without requiring artificial whitespace padding around punctuation.

## 13. Storage model for collections

### Arrays
Each primitive array type has its own node type and contiguous buffer:

```text
iA → int *data, size
fA → float *data, size
cA → char *data, size
bA → int *data, size
```

### Lists (`struct lV`)
Lists are dynamic arrays storing generic `struct Value` items:

```c
struct lV {
    char name[100];
    int size;
    int capacity;
    struct Value *data;
    struct lV *ref;
    struct lV *next;
};
```

- **Dynamic Growth**: Allocation doubles when `size >= capacity`.
- **Reference Aliasing**: When passed to functions, `ref` links the local parameter to the caller's list, enabling in-place reference semantics.

### Maps (`struct mV`)
Maps are associative key-value stores with generic `Value` entries:

```c
struct mapEntry {
    char key[100];
    struct Value value;
};

struct mV {
    char name[100];
    int count;
    int capacity;
    struct mapEntry *entries;
    struct mV *next;
};
```

- **Dynamic Growth**: Entries buffer automatically reallocates (`realloc`) when capacity is exceeded.
- **Generic Values**: Elements can store numbers, strings, lists, or nested maps.

## 14. Modules

`use` opens another `.bykig` file and invokes:

```c
run_interpreter_loop(mod_fp, resolved_path);
```

The interpreter temporarily changes `current_fp` to the module and restores it afterward. Imported code runs in the current environment. Function definitions created while importing retain the module path in `funcDef.filepath`, allowing subsequent calls to reopen the defining file and seek to `bodyPos`.

## 15. Source-position control

The interpreter relies heavily on:

```c
ftell()
fseek()
```

for:
- while-loop and for-loop restart positions
- conditional skipping (`if`, `elif`, `else`)
- function body entry and exit
- module switching
- caller-position restoration

## 16. Self-hosting & `mini-bykig++`

`mini-bykig++` is an interpreter for `.mini` written entirely in BYKIG++, located in `mini-bykig++/`:

- **Lexical Environments** (`memory.bykig`): Uses generic maps (`map scopeStack`, `map envVal`, `map envType`) to implement lexically-scoped variables and function registries.
- **Heap Management**: Allocates virtual object IDs (`newList`, `newMap`) and maps object operations to associative storage.
- **Expression Evaluation** (`evaluator.bykig`): Builds expression strings and leverages BYKIG++'s built-in `eval()` and `quote()`.
- **Parsing** (`main.bykig`): Uses `file_scan_*` built-ins to tokenize `.mini` scripts and drive control-flow and statement execution.

## 17. Error handling

`safe_fscanf()` turns required-token read failures into a fatal error instead of allowing execution to continue with missing input.

Other runtime checks include:
- undefined variable lookup
- invalid function/builtin lookup
- call-stack overflow protection
- break/continue outside loops
- collection index out-of-bounds protection
- file open/read/write failures

## 18. Practical development checklist

Before considering an interpreter feature complete, test:
1. Normal execution
2. Nested execution (nested loops, nested collections, nested expressions)
3. Lexical scoping and variable shadowing
4. Function parameter passing (value copy vs. reference)
5. Recursion and frame cleanup
6. Collection mutation (`push`, `pop`, `delete`, index assignment)
7. Error paths and bounds checks
8. Full regression suite (`python tests/run_tests.py`)
