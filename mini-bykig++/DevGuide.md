# mini-bykig++ Developer Guide

This guide details the internal runtime architecture, memory models, execution lifecycle, and extension points of **`mini-bykig++`**. It is intended for contributors, maintainers, and developers looking to extend the self-hosted interpreter.

---

## 1. Architecture Overview

`mini-bykig++` is an interpreter written 100% in pure BYKIG++. It interprets the `.mini` scripting language using token-driven evaluation, runtime memory registries, and explicit call/environment stacks.

```text
                     .mini Source File
                            │
                            ▼
              ┌───────────────────────────┐
              │        main.bykig         │  CLI & Main Execution Loop
              │   Statement Dispatcher    │  Control Flow (if, while, for)
              └─────────────┬─────────────┘
                            │
         ┌──────────────────┴──────────────────┐
         ▼                                     ▼
┌───────────────────────────┐       ┌───────────────────────────┐
│      evaluator.bykig      │       │       memory.bykig        │
│   Expression Evaluator    │       │   Lexical Environment     │
│   Collection Parser       │◄─────►│   Heap Storage (List/Map) │
│   Member Dot Resolver     │       │   Class Registry & Heap   │
│   Method/Func Invocation  │       │   Receiver (this) Stack   │
└───────────────────────────┘       └───────────────────────────┘
```

---

## 2. Source Layout & Responsibilities

The codebase is partitioned into three decoupled modules:

| Module | File | Core Responsibilities |
|---|---|---|
| **Memory & Storage** | [`memory.bykig`](memory.bykig) | Lexical environment stack, variable tables, heap storage for dynamic lists/maps, object instance storage, class registry, receiver stack (`this`), and recursive string formatting. |
| **Evaluator** | [`evaluator.bykig`](evaluator.bykig) | Infix expression evaluation, operator precedence, collection literal parsing, nested index resolution, member dot access, built-in dispatch, and function/method/constructor execution. |
| **Execution Loop** | [`main.bykig`](main.bykig) | Interactive CLI, file scanner integration, token-driven statement dispatch, control flow (`if`/`elif`/`else`, `while`, `for`), block skipping, and member assignment. |

---

## 3. Lexical Scoping & Variable Environment

Lexical environments in `mini-bykig++` use a stacked model managed in [`memory.bykig`](memory.bykig).

### Scope Stack Architecture
- `scopeStack[scopeTop]`: Tracks the base variable index for each scope level.
- `pushScope()`: Pushes a new scope frame when entering a block (`{`).
- `popScope()`: Releases all variables defined within the innermost scope when exiting a block (`}`).
- `scopeTop`: Current depth of the environment stack.

### Variable Tables
Variables are stored using parallel lists indexed by integer IDs:
- `varName[id]`: Identifier string.
- `varType[id]`: Type tag (`0` = Number, `1` = String, `2` = List, `3` = Map, `4` = Object).
- `varValNum[id]`: Numeric value (stores float/int, boolean `0`/`1`, or heap index for lists, maps, and objects).
- `varValStr[id]`: String payload (when `varType == 1`).

### Variable Resolution & Mutation
- `findVar(name)`: Searches linearly from `varCount - 1` down to `0`. Because new variables are pushed to the end, inner variables naturally shadow outer variables of the same name.
- `defineVar(name, type, numVal, strVal)`: Registers a variable in the active scope.
- `setVar(name, type, numVal, strVal)`: Mutates an existing variable located via `findVar`.

---

## 4. Heap Storage Subsystems

Dynamic collections and object instances are allocated on centralized heaps in [`memory.bykig`](memory.bykig). Variables store integer heap indices referencing these allocations.

### Dynamic Lists
Lists are dynamically resizable arrays:
- `heapListCap[listId]`: Allocated capacity.
- `heapListLen[listId]`: Current element count.
- `heapListValNum[listId][elemIdx]`: Numeric or heap-reference value.
- `heapListValStr[listId][elemIdx]`: String value.
- `heapListValType[listId][elemIdx]`: Element type tag.
- Operations: `newHeapList()`, `heapListPush()`, `heapListPop()`.

### Generic Associative Maps
Maps store key-value associations:
- `heapMapCap[mapId]`: Capacity.
- `heapMapLen[mapId]`: Number of active keys.
- `heapMapKeys[mapId][entryIdx]`: String key names.
- `heapMapValNum[mapId][entryIdx]`: Stored numeric value or heap pointer.
- `heapMapValStr[mapId][entryIdx]`: Stored string value.
- `heapMapValType[mapId][entryIdx]`: Value type tag.
- Operations: `newHeapMap()`, `heapMapSet()`, `heapMapGet*()`, `heapMapExists()`, `heapMapDelete()`.

### Object Instances
Object instances store dynamic state and field tables:
- `objClass[objId]`: Name of the object's runtime class.
- `objFieldVal[objId][fieldName]`: Numeric or heap-reference value of the field.
- `objFieldValStr[objId][fieldName]`: String value of the field.
- `objFieldType[objId][fieldName]`: Type tag of the field.
- Operations:
  - `newObjectInstance(className)`: Allocates an instance and increments `objCount`.
  - `initObjectFields(objId, className)`: Recursively initializes all inherited and declared fields to default values (`0`, `""`, or `null`).
  - `setObjectField(objId, fieldName, type, numVal, strVal)`.
  - `getObjectFieldVal(objId, fieldName)`.

### Formatted Value Representation
`formatValue(type, numVal, strVal)` recursively produces the string output for any runtime value:
- `0` (Number) -> String of `numVal`.
- `1` (String) -> `strVal`.
- `2` (List) -> Formatted bracket string: `[elem1, elem2, ...]`.
- `3` (Map) -> Formatted brace string: `{"key1": val1, "key2": val2}`.
- `4` (Object) -> `<Object ClassName#id>` (or `"null"` if `numVal == 0`).

---

## 5. OOP Class Registry & Virtual Dispatch

The OOP subsystem maintains metadata tables in [`memory.bykig`](memory.bykig) to manage class definitions, inheritance, and dynamic dispatch.

### Class Metadata Tables
- `classExists[className]`: Boolean existence check.
- `classParent[className]`: Direct superclass name (`""` if root).
- `classFieldCount[className]`, `classFields[className][i]`: Field registry.
- `classFieldPrivate[className][fieldName]`: Boolean visibility (`1` = private, `0` = public).
- `classFieldOwner[className][fieldName]`: The class where the field was declared.
- `classMethodCount[className]`, `classMethods[className][i]`: Method registry.
- `classMethodPos[className][methodName]`: Source file offset (`ftell`) of the method body.
- `classMethodParamCount[className][methodName]`: Parameter count.
- `classMethodParams[className][methodName][i]`: Parameter names.
- `classMethodPrivate[className][methodName]`: Visibility.
- `classMethodOwner[className][methodName]`: Declaring class name.
- `classHasCtor[className]`: Boolean constructor flag.
- `classCtorPos`, `classCtorParamCount`, `classCtorParams`, `classCtorPrivate`: Constructor metadata.

### Hierarchy & Encapsulation Helpers
- `isSubclass(derived, base)`: Traverses `classParent` up to the root to verify inheritance relationships.
- `findMethodClass(className, methodName)`: Traverses upwards starting at `className` to locate the class implementing `methodName`. Enables virtual dispatch and method overriding.
- `findFieldDeclClass(className, fieldName)`: Traverses upwards to locate the class declaring `fieldName`.
- `canAccessField(className, fieldName, execClass)`:
  - If field is public: returns `1`.
  - If field is private: returns `1` only if `execClass == declClass`. Otherwise returns `0`.

### Dynamic Receiver Stack (`this`)
To guarantee reentrancy and prevent receiver leakage across nested calls, recursion, and free functions:
- `thisObjStack[thisTop]`: Stores the active receiver's heap object ID.
- `thisExecClassStack[thisTop]`: Stores the executing method's declaring class.
- `pushThis(objId, execClass)`: Pushes a receiver context when entering a method or constructor.
- `popThis()`: Restores the caller's receiver context upon returning.
- `execute_func_call` pushes `(0, "")` to guarantee that global free functions never inherit an active receiver context.

---

## 6. Expression & Member Evaluation

[`evaluator.bykig`](evaluator.bykig) provides expression parsing and member resolution.

### Expression Evaluation Lifecycle
1. `evalExpressionUntil(delim)`: Evaluates infix tokens until encountering `delim` (e.g., `;`, `,`, `]`, `)`).
2. `evalArg(token)`:
   - Resolves numeric literals, string literals, and booleans.
   - Handles `new ClassName ( args )` expressions.
   - Detects collection literals: `[` triggers `parse_list_literal` and `{` triggers `parse_map_literal`.
   - Resolves member accesses (`token ~ "."`) via `resolve_member_access`.
   - Resolves nested collection indexing (`token ~ "["`) via `resolve_indexed_val`.
   - Resolves identifiers from the lexical scope via `findVar`.
   - Recognizes `this` as a receiver reference.
3. Operator application applies arithmetic, relational, and logical operations with standard operator precedence.

### Member Dot Resolution (`resolve_member_access`)
Handles chained dot notation:
- Splits target into receiver base and member: `obj . member`.
- If `member` is followed by `(`, dispatches `execute_method_call(objId, methodName)`.
- Otherwise, verifies field access permissions via `canAccessField`, reading the field value from `objFieldVal` / `objFieldValStr`.
- Supports nested object chaining: `company . owner . name`.

### Method & Constructor Execution
- **Constructor Execution** (`execute_class_constructor`):
  1. Checks constructor access permissions.
  2. Evaluates arguments in the caller's scope.
  3. Records current file position (`file_scan_ftell`).
  4. Seeks to constructor position (`classCtorPos`).
  5. Pushes `this` receiver and pushes a new lexical scope.
  6. Binds `this` as an object variable in scope.
  7. Binds argument values to constructor parameter names.
  8. Executes the constructor block using `run_block()`.
  9. Pops lexical scope and pops `this` receiver.
  10. Restores original file position.
- **Method Execution** (`execute_method_call`):
  1. Resolves declaring class via `findMethodClass(objClass[objId], methodName)`.
  2. Enforces access control against caller's `getThisExecClass()`.
  3. Evaluates arguments and records file return position.
  4. Pushes `this` receiver and pushes new scope.
  5. Binds parameter names to argument values.
  6. Executes method body via `run_block()`.
  7. Preserves return values in `retValNum`, `retValStr`, `retValType`.
  8. Pops scope and pops `this` receiver.
  9. Restores file return position.

---

## 7. Execution Engine (`main.bykig`)

[`main.bykig`](main.bykig) contains the outer token loop and control flow handlers.

### Statement Dispatch
The main loop consumes tokens and dispatches:
- `class`: Parses class declarations, fields, constructors, and methods, recording positions and skipping bodies via `skip_block()`.
- `var` / primitive types / class names: Parses variable declarations and evaluates initializers.
- `if` / `elif` / `else`: Evaluates conditions; executes matching branch via `run_block()` and skips non-matching branches via `skip_block()`.
- `while`: Records loop start position; evaluates condition; executes body; restores start position if true, or skips block if false.
- `for`: Evaluates initializer; loops while condition is true, evaluating the step statement after each iteration.
- `out` / `in`: Handles standard I/O.
- Member mutation (`obj . field = expr ;`): Resolves receiver, checks private write permissions, evaluates RHS, and updates field storage.
- Standalone method / function calls: Invokes methods or functions with discarded return values.

---

## 8. Critical Host Runtime Invariant

> [!IMPORTANT]
> **BYKIG++ Block Formatting Invariant**
> In the native host interpreter (`Interpreter.c`), conditional block skipping scans line-by-line using `fgets` expecting `{` and `}` on separate lines.
> **All `.bykig` code within `mini-bykig++` must use standard multi-line formatting for control flow:**
> ```bykig
> if condition
> {
>     statement ;
> }
> ```
> Placing braces on the same line as statements (e.g., `if c { stmt ; }`) causes the host block scanner to miss the `{`, losing block depth synchronization and resulting in premature EOF or infinite parsing loops.

