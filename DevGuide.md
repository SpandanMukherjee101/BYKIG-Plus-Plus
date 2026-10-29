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
├── tests/                   # Regression test suites and automated test runners
│   ├── run_tests.py         # Full regression suite test runner (173 tests)
│   ├── run_oop_tests.py     # Dedicated OOP subsystem conformance harness (46 tests)
│   ├── basics/              # Core types, expressions, shadowing, strings
│   ├── control_flow/        # Conditionals, while loops, for loops, break/continue
│   ├── data_structures/     # Arrays, dynamic lists, generic maps, collection references
│   ├── functions/           # Function calls, recursion, scope, return values
│   ├── modules/             # File imports, circular dependency tests
│   ├── integration/         # Sorting, mock database, prime sieve, file analyzers
│   └── oop/                 # Complete OOP test suite organized by subsystem:
│       ├── classes/         # Class declaration, duplicate/malformed validation
│       ├── instances/       # Instantiation, identity, null references, aliases
│       ├── fields/          # Instance field access, typing, collections as fields
│       ├── methods/         # Instance methods, params, recursion, module methods
│       ├── this/            # Receiver binding, call isolation, self returns
│       ├── constructors/    # Explicit construct methods, default ctor, errors
│       ├── access_control/  # Public and private member encapsulation
│       ├── inheritance/     # Single inheritance, multilevel, duplicate guards
│       ├── overriding/      # Dynamic virtual dispatch, signature compatibility
│       ├── type_checks/     # is_instance runtime checks across hierarchies
│       └── audit/           # Memory safety, cyclic references, capacity bounds
└── mini-bykig++/            # Self-hosted companion interpreter written in BYKIG++
    ├── main.bykig           # CLI loop, statement dispatch, block execution
    ├── memory.bykig         # Lexical environments, heap lists/maps, OOP registry
    ├── evaluator.bykig      # Expressions, collection literals, nested indexing, OOP dispatch
    ├── README.md            # Companion interpreter overview and landing page
    ├── UserGuide.md         # Syntax, types, collections, OOP, and user guide
    ├── DevGuide.md          # Internal architecture, memory layout, and runtime invariants
    ├── FeatureMatrix.md     # Feature support and runtime boundaries matrix
    └── tests/               # Test suites for mini-bykig++
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
    VAL_ERROR,
    VAL_OBJECT
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
VAL_OBJECT     → ptr points to struct objInstance
array types    → ptr points to typed array struct
VAL_ERROR      → error state
```

The `Value` object is the runtime transport format. String conversion is handled via `value_to_string(struct Value v, char *dest, size_t dest_sz)`. For objects, `value_to_string()` produces `<Object ClassName#id>` or `null`.

## 5. Environment model

An environment is represented by the storage pointers inside `callFrame`:

```text
IV/FV/CV/BV/SV   (primitive variables)
IA/FA/CA/BA      (typed arrays)
LV/MV            (dynamic lists and generic maps)
OV               (object instance reference variables)
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

## 14. Object-Oriented Runtime Architecture

The OOP subsystem adds classes, instances, encapsulation, single inheritance, and virtual dispatch while strictly preserving the interpreter's direct, token-driven architecture.

### 14.1 Class Declaration and Registration (`CLASS_REGISTRY`)

Classes are declared using `class Name [extends SuperName] { ... }` and registered in the global linked list `CLASS_REGISTRY`:

```c
struct classDef {
    char name[100];
    char parentName[100];
    struct classDef *parent;
    struct classField fields[MAX_CLASS_FIELDS];
    int fieldCount;
    struct classMethod methods[MAX_CLASS_METHODS];
    int methodCount;
    struct funcDef *constructor;
    struct classDef *next;
};
```

- **Single Inheritance Order**: A class may extend at most one superclass. The superclass must be declared prior to the derived class in execution order. Unknown superclasses, self-inheritance (`class A extends A`), and cyclic inheritance chains are rejected with clear diagnostics.
- **Rollback on Parsing Failure**: If a class body contains malformed definitions or duplicate fields, class registration rolls back: allocated method descriptors are freed, field allocations are rolled back, and the incomplete class is unlinked from `CLASS_REGISTRY`, leaving no dangling metadata.
- **Capacity Bounds**: Enforces up to `MAX_CLASS_FIELDS` (64) fields and `MAX_CLASS_METHODS` (64) methods per class. Attempting to declare or inherit members beyond these limits emits a descriptive diagnostic and cleanly aborts registration to prevent memory corruption.
- **Field Collision Prevention**: A class cannot declare duplicate field names within its own body, nor can it declare a field whose name collides with an inherited field from its superclass.

### 14.2 Member Visibility and Encapsulation

Fields and methods support access control via `public` and `private` visibility specifiers:

- **Default Member Visibility**: If an explicit visibility modifier is omitted, members default to `public`.
- **Declaring-Class Context Model**: Access control is enforced against the member's `declaringClass` rather than the runtime class of the instance:
  ```c
  struct classDef *callerContext = (callStackTop >= 0) ? callStack[callStackTop].executingClass : NULL;
  struct classDef *declaringClass = member->declaringClass ? member->declaringClass : inst->cls;
  if (callerContext != declaringClass) {
      // Access blocked: cannot access private member from outside declaring class
  }
  ```
- **Base-Class Access Preservation**: When a base-class method executes on a derived instance, it retains full access to the base class's private fields.
- **Derived-Class Protection**: Derived-class methods cannot directly read, write, or invoke private members introduced by a superclass.
- **Same-Class Cross-Instance Access**: Methods of class `A` may access private members on another instance of class `A` (e.g., `other.privateField`).
- **Global Function Isolation**: Global functions and script top-level statements run with `callerContext == NULL` and cannot access private members.

### 14.3 Constructor Resolution and `super.construct(...)`

- **Constructor Resolution**: Constructors are declared using `public func construct(...)`. A class has at most one constructor. Constructors are not inherited as callable signatures.
- **Default Constructor**: If a class declares no explicit constructor, it provides an implicit zero-argument default constructor, provided all superclasses in its inheritance chain can be initialized without required arguments.
- **Explicit Parent Chaining**: Derived constructors invoke parent constructors using `super.construct(arg1, arg2, ...);`.
- **`super.construct` Contract Rules**:
  1. `super.construct(...)` must be the first executable statement in a derived constructor body.
  2. `super` is strictly reserved for constructor chaining; calling `super.construct` outside a constructor or on a class without a superclass is rejected.
  3. Parent constructors execute against the same object identity (`this`), traversing the hierarchy in base-to-derived order.
  4. If a superclass constructor requires arguments and the derived constructor neither calls `super.construct(...)` nor supplies valid parameters, instantiation fails with a diagnostic and returns `null`.

### 14.4 Receiver (`this`) and Dynamic Virtual Dispatch

- **Receiver Binding**: Method calls dynamically bind `this` to the receiver instance `struct objInstance*` in `callStack[callStackTop].thisObj` and local environment `envStack[envTop]`.
- **Call Frame Isolation & Restoration**: The receiver and `executingClass` context are saved per call frame. Normal return, early return (`return;`), nested method calls (`a.foo(b.bar())`), and recursive calls immediately restore the caller's receiver context upon frame pop.
- **Free Function Isolation**: Free function calls explicitly push frames with `thisObj = NULL` and `executingClass = NULL`, preventing receiver leakage.
- **Dynamic Method Dispatch**: Method resolution starts at the receiver's actual dynamic runtime class (`inst->cls`) and traverses parent links upward. Inherited methods remain callable if not overridden.

### 14.5 Override Signature Validation

When a derived class declares a method with the same name as an inherited method:
- **Parameter Count**: Must match the base method's parameter count exactly.
- **Parameter Types**: Parameter types must match identically, including typed object parameter types (`funcParam.className`).
- **Visibility Preservation**: An override cannot reduce visibility (overriding a `public` base method as `private` is rejected).
- Signature violations emit clear diagnostics during class registration and discard the invalid override.

### 14.6 Subtyping, Type Compatibility, and Predicates

- **Unified Subtyping Rule**: Variable assignment, function/method parameter passing, and field assignment enforce `isSubclassOf(runtimeCls, targetCls)`. Passing or assigning an incompatible instance produces a clear type mismatch diagnostic and sets the target to `null`.
- **`is_instance(obj, "ClassName")`**: Built-in function returning `1.0` if `obj`'s runtime class matches `ClassName` or descends from it. Returns `0.0` for non-matching classes, unknown classes, primitive values, and `null` references.
- **Null Safety**: Uninitialized object variables and `null` references (`v.type == VAL_OBJECT && v.ptr == NULL`) format as `"null"`. Reading or writing fields or calling methods on null receivers produces safe diagnostics without crashing. The built-in `is_null(expr)` returns `1.0` for null references.

### 14.7 Object Lifetimes, Aliasing, and Centralized Cleanup

- **Heap Ownership**: Object instances (`struct objInstance`) are tracked on a central intrusive heap `OBJECT_HEAP`.
- **Reference Semantics**: Object variables and fields hold non-owning references. Normal scope and function frame exits (`cleanupFrame`) free variable binding descriptors (`struct oV`) without deallocating heap instances.
- **Collection Aliasing**: Storing an object into a `list` or `map` preserves the instance reference. Mutating an object retrieved from a collection (`inventory[0].price = 99.0;`) updates the object in-place across all aliases.
- **Deterministic Teardown**: `freeAllObjects(&OBJECT_HEAP)` linearly frees all heap instances and field storage at shutdown. Because user-level cyclic references (`n1.next = n2; n2.next = n1;`) do not affect the internal heap linked list, shutdown deallocation never loops indefinitely.

### 14.8 Explicitly Unsupported OOP Features (Non-Goals)

To prevent specification drift, the following features are explicitly unsupported in BYKIG++:
- **Interfaces / Protocols**: No `interface` or `implements` keywords.
- **Multiple Inheritance**: Strictly single inheritance (`extends` exactly one class).
- **Abstract Classes / Pure Virtual Methods**: No `abstract` keyword.
- **General `super.method()` Dispatch**: Calling parent methods outside constructor chaining (`super.construct`) is unsupported.
- **Destructors / Finalizers**: Objects have no custom destructor or finalizer hooks.
- **Garbage Collection / Reference Counting**: Heap objects persist until interpreter shutdown.
- **Operator Overloading**: Custom operator definitions on classes are not supported.

## 15. Modules

`use` opens another `.bykig` file and invokes:

```c
run_interpreter_loop(mod_fp, resolved_path);
```

The interpreter temporarily changes `current_fp` to the module and restores it afterward. Imported code runs in the current environment. Function definitions created while importing retain the module path in `funcDef.filepath`, allowing subsequent calls to reopen the defining file and seek to `bodyPos`.

## 16. Source-position control

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

## 17. Self-hosting & `mini-bykig++`

`mini-bykig++` is an interpreter for `.mini` written entirely in BYKIG++, located in `mini-bykig++/`:

- **Lexical Environments** (`memory.bykig`): Uses generic maps (`map scopeStack`, `map envVal`, `map envType`) to implement lexically-scoped variables and function registries.
- **Heap Management**: Allocates virtual object IDs (`newList`, `newMap`) and maps object operations to associative storage.
- **Expression Evaluation** (`evaluator.bykig`): Builds expression strings and leverages BYKIG++'s built-in `eval()` and `quote()`.
- **Parsing** (`main.bykig`): Uses `file_scan_*` built-ins to tokenize `.mini` scripts and drive control-flow and statement execution.

## 18. Error handling

`safe_fscanf()` turns required-token read failures into a fatal error instead of allowing execution to continue with missing input.

Other runtime checks include:
- undefined variable lookup
- invalid function/builtin lookup
- call-stack overflow protection
- break/continue outside loops
- collection index out-of-bounds protection
- file open/read/write failures
- null receiver member access guards
- private member access violation guards
- class and method capacity limits

## 19. Practical development checklist

Before considering an interpreter feature complete, test:
1. Normal execution
2. Nested execution (nested loops, nested collections, nested expressions)
3. Lexical scoping and variable shadowing
4. Function parameter passing (value copy vs. reference)
5. Recursion and frame cleanup
6. Collection mutation (`push`, `pop`, `delete`, index assignment)
7. OOP features (classes, constructors, methods, `this`, access control, inheritance, polymorphism)
8. Error paths and bounds checks
9. Full regression suite (`python tests/run_tests.py`)
