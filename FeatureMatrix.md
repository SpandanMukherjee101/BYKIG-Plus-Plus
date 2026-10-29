# BYKIG++ Feature Matrix

## Core language features

| Feature | Status | Notes |
|---|:---:|---|
| `int` | ✅ | Stored as `int`; exposed to the expression engine through `Value`. |
| `float` | ✅ | Native floating-point runtime type. |
| `char` | ✅ | Stored as `char`; expressions expose it numerically. |
| `bool` | ✅ | Stored as `int`; truth values are represented numerically. |
| `string` | ✅ | Fixed-size character buffer per variable; string evaluation preserves spaces and quotes. |
| Variable declaration | ✅ | Supported in the active environment. |
| Variable assignment | ✅ | RHS is evaluated through the expression engine. |
| Conditional block scope | ✅ | Variables declared in executed `if` / `elif` / `else` blocks are released at the closing brace. |
| Expression evaluation | ✅ | Arithmetic, comparison (`~`, `!`, `<`, `>`, `<=`, `>=`), logical operators, parentheses, and complex resolution. |
| `if` / `elif` / `else` | ✅ | Source-position/block based control flow. |
| `while` | ✅ | Loop position is retained so the body can be re-entered. |
| `for` | ✅ | Loop initialization, condition, and iteration statements; loop-scoped variable support; works with nested loops, `break`, and `continue`. |
| `break` | ✅ | Breaks the active loop. |
| `continue` | ✅ | Skips the current iteration and resumes loop control. |
| `func` | ✅ | Functions are registered by name, parameter metadata, source position, and file path. |
| `return` | ✅ | Produces a runtime `Value` for the active call. |
| Recursion | ✅ | Implemented through the call stack and per-call environments. |
| Module import with `use` | ✅ | Executes another `.bykig` file in the current runtime environment. |
| Comments | ✅ | `//` single-line comments. |
| Program terminator | ✅ | `.` stops execution early; reaching EOF also ends a program successfully. |

## Object-Oriented Programming (OOP)

| Feature | Status | Notes |
|---|:---:|---|
| `class` declaration | ✅ | Registers class metadata, fields, and methods in `CLASS_REGISTRY`; rolls back safely on malformed body. |
| Single inheritance (`extends`) | ✅ | Derived class inherits fields and methods in parent-to-child order; superclass must precede subclass. |
| Inheritance validation | ✅ | Duplicate class names, unknown parents, self-inheritance, cyclic inheritance, and field collisions are rejected. |
| Member capacity limits | ✅ | Strictly enforces up to 64 fields (`MAX_CLASS_FIELDS`) and 64 methods (`MAX_CLASS_METHODS`) per class. |
| Instance fields | ✅ | Supported types include primitives, strings, arrays, lists, maps, and objects; typed object fields enforce subclass compatibility. |
| Access control | ✅ | Explicit `public` and `private` modifiers; default visibility is public; access enforced against declaring class context. |
| Cross-instance private access | ✅ | Methods of a class can access private fields on another instance of the exact same class (`other.secret`). |
| `new` instantiation | ✅ | Heap-allocates `struct objInstance`, initializes default field values, and executes constructor. |
| `construct` constructor | ✅ | Explicit constructor method; at most one constructor per class; argument count and types strictly validated. |
| Default constructor | ✅ | Implicit zero-argument constructor provided when omitted, only if parent chain requires no constructor arguments. |
| `super.construct(...)` | ✅ | Explicit parent constructor invocation; must be the first statement in derived constructor; multilevel chaining supported. |
| Instance methods | ✅ | Scoped per declaring class; callable via `obj . method ( args )` or `obj.method ( args )`. |
| `this` reference | ✅ | Receiver dynamically bound in call frame and local environment; reentrant and isolated across nested/recursive calls. |
| Method overriding & dynamic dispatch | ✅ | Dispatches dynamically based on receiver's runtime class; un-overridden inherited methods remain callable. |
| Override signature validation | ✅ | Parameter counts and parameter types must match base method (including object types); visibility reduction rejected. |
| Assignment compatibility | ✅ | Variables, parameters, and fields enforce `isSubclassOf(child, parent)`; incompatible assignments rejected. |
| `is_instance()` predicate | ✅ | Built-in testing exact class match or any descendant class in hierarchy; returns 0 on null, primitives, and invalid classes. |
| `is_null()` predicate | ✅ | Built-in testing whether an object reference is null or uninitialized. |
| Null dereference safety | ✅ | Guarded member access on uninitialized or null object references with explicit error diagnostics. |
| Reference semantics & aliasing | ✅ | Variables and fields hold non-owning heap pointers; mutations reflect across all aliases. |
| Collections containing objects | ✅ | `list` and `map` store and return object references; in-place mutations reflect across collection references. |
| Multi-hop navigation | ✅ | Chained member access and collection indexing (`inventory[0].price`, `matrix[0][1]`) resolve to convergence. |
| Centralized heap reclamation | ✅ | Global intrusive linked list `OBJECT_HEAP` cleaned up deterministically at shutdown via `freeAllObjects()`; cycles cannot hang shutdown. |

## Unsupported OOP Features (Explicit Non-Goals)

| Feature | Status | Notes |
|---|:---:|---|
| Interfaces / Protocols | ❌ | No `interface` or `implements` keywords. |
| Multiple inheritance | ❌ | Strictly single inheritance via `extends`; diamond inheritance is not supported. |
| Abstract classes / pure virtual methods | ❌ | No `abstract` keyword or abstract method declarations. |
| General `super.method()` dispatch | ❌ | `super` can only be used as `super.construct(...)` in constructors; calling overridden parent methods from general methods is unsupported. |
| Destructors / Finalizers | ❌ | No destructor hooks (`~ClassName()` or `destruct`); instances have no user-level cleanup routines. |
| Garbage collection / Reference counting | ❌ | Allocated object instances persist on `OBJECT_HEAP` until interpreter termination. |
| Operator overloading | ❌ | Custom operator methods on classes are unsupported. |

## Runtime data structures

| Runtime component | Status | Implementation idea |
|---|:---:|---|
| Unified `Value` | ✅ | `ValueType` (`VAL_FLOAT`, `VAL_STRING`, `VAL_OBJECT`, arrays, lists, maps), scalar storage, string buffer, or `void *ptr`. |
| Environment stack | ✅ | `envStack[10000]`; active scope is selected with `envTop`. |
| Call stack | ✅ | `callStack[10000]`; stores `thisObj`, `executingClass`, return position, return values, caller's `envTop`. |
| Function registry | ✅ | Linked list of `funcDef` structures (`FN`). |
| Class registry | ✅ | Linked list of `classDef` structures (`CLASS_REGISTRY`). |
| Object heap | ✅ | Intrusive singly-linked list of `objInstance` structures (`OBJECT_HEAP`). |
| Typed primitive storage | ✅ | Separate linked lists for `int`, `float`, `char`, `bool`, `string`. |
| Object variable storage | ✅ | Linked list of `struct oV` references per environment frame. |
| Typed arrays | ✅ | Separate structures for int/float/char/bool arrays. |
| Dynamic lists | ✅ | Size/capacity based dynamic `Value` list with capacity doubling. |
| Maps | ✅ | Generic `Value`-based map with dynamic capacity (`realloc`), supporting strings, floats, ints, lists, maps, and objects. |
| Environment cleanup | ✅ | `cleanupFrame()` frees variable link nodes, arrays, list buffers, and maps in the frame without double-freeing objects. |
| Heap cleanup | ✅ | `freeAllObjects(&OBJECT_HEAP)` frees all allocated object instances and field arrays at shutdown. |

## Collections

| Feature | Status | Notes |
|---|:---:|---|
| `intarr` | ✅ | Fixed-size integer array. |
| `floatarr` | ✅ | Fixed-size float array. |
| `chararr` | ✅ | Fixed-size character array. |
| `boolarr` | ✅ | Fixed-size boolean/integer array. |
| `list` | ✅ | Dynamic collection backed by `Value *data` with capacity doubling; elements can hold any current runtime `Value` (numbers, strings, nested lists, maps). |
| `map` | ✅ | Generic dynamic collection with string/numeric keys and arbitrary `Value` elements; dynamic capacity growth (`realloc`). |
| `arr[index]` | ✅ | Typed array access. |
| `list[index]` | ✅ | Numeric indexed access. |
| `map[key]` | ✅ | Key-based lookup/update. |
| Nested indexing | ✅ | Multi-level indexing for expressions and assignments (e.g. `matrix[0][1] = 99 ;`, `data["users"][0]["name"]`). |
| Collection literals | ✅ | Lists, maps, and typed arrays accept `{ ... }` or `[ ... ]` initializers. |
| Function collection arguments | ✅ | Lists are passed by reference; maps and arrays are copied into function-local parameters. |
| `push()` | ✅ | `push(list, value)` appends to a list; `push(map, key, value)` inserts or updates a map entry. |
| `pop()` | ✅ | `pop(list)` removes the final list value; `pop(map, key)` removes and returns a map value. |

## Built-ins

| Built-in | Status | Purpose |
|---|:---:|---|
| `len()` | ✅ | Length of supported strings/arrays/lists/maps. |
| `push()` | ✅ | Appends a list value or inserts/updates a map key. |
| `pop()` | ✅ | Removes a list value or a map key and returns its value. |
| `exists()` | ✅ | `exists(name)` tests whether a runtime value exists; `exists(map, key)` tests a map key. |
| `delete()` | ✅ | `delete(name)` removes a runtime value; `delete(map, key)` removes a map key. |
| `quote()` | ✅ | Wraps a string in double quotes for expression serialization or nested JSON-like formatting. |
| `strip_quotes()` | ✅ | Removes outer double quotes from a string value. |
| `is_num()` | ✅ | Returns `1` if the string/value represents a valid number, `0` otherwise. |
| `is_str()` | ✅ | Returns `1` if the value is a string, `0` otherwise. |
| `starts_with()` | ✅ | `starts_with(str, prefix)` tests if string starts with prefix. |
| `parse_float()` | ✅ | Converts a string/numeric value to a float result. |
| `is_instance()` | ✅ | Tests if an object instance belongs to a class or any of its derived subclasses. |
| `file_exists()` | ✅ | Tests whether a file can be opened for reading. |
| `file_write()` | ✅ | Overwrites a file. |
| `file_append()` | ✅ | Appends to a file. |
| `file_read()` | ✅ | Reads a file into the built-in string-return path. |
| `file_scan_open()` | ✅ | Opens persistent scanner state. |
| `file_scan_next()` | ✅ | Reads the next token; whitespace-delimited words and self-delimiting punctuation (`[`, `]`, `{`, `}`, `(`, `)`, `,`, `;`) are separated automatically. |
| `file_scan_is_str()` | ✅ | Returns `1` if the most recent token from `file_scan_next()` was a quoted string literal. |
| `file_scan_close()` | ✅ | Closes scanner state. |
| `file_scan_get_pos()` | ✅ | Returns scanner file position. |
| `file_scan_set_pos()` | ✅ | Restores scanner file position. |

## I/O and modules

| Feature | Status | Notes |
|---|:---:|---|
| Console output `out` | ✅ | Supports strings, numbers, collections, and formatting sequences. |
| Console input `in` | ✅ | Reads into supported variables. |
| `%n` | ✅ | Newline output sequence. |
| `%"` | ✅ | Double-quote output sequence. |
| `%+` | ✅ | Plus-sign output sequence. |
| Module path resolution | ✅ | Direct path first, then path relative to current source file. |
| Shared module environment | ✅ | Imported code runs without creating a separate module namespace. |
| Function source-file tracking | ✅ | `funcDef.filepath` records the defining file. |

## Important implementation boundaries

| Area | Current behavior |
|---|---|
| Expression representation | Direct postfix stack evaluation with prior resolution of functions, builtins, and nested indexing; no AST. |
| Parser/lexer | Direct token scanner with punctuation delimitation; no traditional AST compiler. |
| Lists | Generic `Value`-backed dynamic arrays with capacity doubling; supports nested indexing and arbitrary `Value` elements. |
| Maps | Generic `Value`-backed dynamic entries (`mapEntry *entries`) with automatic capacity expansion; supports nested indexing. |
| String evaluation | Handled through `format_string_token()` and postfix literal unescaping, preserving spaces and quotes cleanly. |
| Modules | Execute into the current environment rather than a separate namespace. |
| Module cache/cycle system | No dedicated import cache or namespace graph. |
| Function registry lifetime | Registry nodes are process-lifetime allocations in the current source. |
