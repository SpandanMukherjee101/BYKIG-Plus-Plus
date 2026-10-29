# mini-bykig++ Feature Matrix

This document provides a comprehensive status matrix of language features, collections, object-oriented capabilities, built-in functions, and runtime components supported by **`mini-bykig++`**.

---

## 1. Core Language Features

| Feature | Status | Notes |
|---|:---:|---|
| Numbers (Integer & Float) | ✅ | Floating point and integer arithmetic; native numeric evaluation. |
| Strings | ✅ | Quoted string literals; supports whitespace, escaped quotes, and string concatenation with `+`. |
| Booleans | ✅ | `true` and `false` literals; resolve to `1` and `0` in arithmetic/relational contexts. |
| Variable Declaration (`var`) | ✅ | Dynamically-typed declarations; scoped to the active lexical environment. |
| Typed Variable Declarations | ✅ | `int`, `float`, `string`, `bool`, `char`, or registered class name. |
| Uninitialized Declarations | ✅ | `Type varName ;` supported; initialized to default empty/zero value. |
| Variable Assignment | ✅ | Mutates existing variables through linear environment lookup. |
| Infix Arithmetic | ✅ | `+`, `-`, `*`, `/`, `%` with standard operator precedence and grouping `( ... )`. |
| Relational Operators | ✅ | `==`, `!=`, `<`, `>`, `<=`, `>=`. |
| Logical Operators | ✅ | `&&`, `||`, `!`. |
| String Concatenation | ✅ | Auto-coerces numbers, booleans, and variables when concatenated with strings. |
| `if` / `elif` / `else` | ✅ | Arbitrary multi-branch conditional execution. |
| `while` Loops | ✅ | Loop condition re-evaluation via source position tracking. |
| `break` / `continue` | ✅ | Loop interruption and iteration skipping supported within `while` loops. |
| `for` Loops | ✅ | Standard three-clause `for var i = 0 ; i < n ; i = i + 1 { ... }`. |
| Lexical Scoping | ✅ | Block-level environments (`pushScope` / `popScope`). |
| Variable Shadowing | ✅ | Inner block variables cleanly shadow outer variables without overwriting outer state. |
| Functions (`func`) | ✅ | User-defined functions with parameter lists and source-offset tracking. |
| Return Statements (`return`) | ✅ | Immediate return with expression value to caller. |
| Void Procedures | ✅ | Functions without an explicit return statement execute for side-effects. |
| Single Recursion | ✅ | Full call stack support (e.g., `factorial`). |
| Double Recursion | ✅ | Re-entrant call frame isolation (e.g., `fibonacci`). |
| Single-Line Comments (`//`) | ✅ | Scanner ignores all characters following `//` until newline. |
| Semicolon Delimiters (`;`) | ✅ | Statements terminated by spaced semicolon. |

---

## 2. Dynamic Collections & Nested Indexing

| Feature | Status | Notes |
|---|:---:|---|
| Dynamic Lists (`[ ... ]`) | ✅ | Heterogeneous elements (numbers, strings, booleans, collections, objects). |
| List Bracket Indexing | ✅ | Zero-based lookup: `lst[0]`. |
| List In-Place Mutation | ✅ | Assignment by index: `lst[1] = 99 ;`. |
| Generic Associative Maps (`{ ... }`) | ✅ | Key-value associative pairs with string or numeric keys. |
| Map Bracket Lookup | ✅ | Lookup by key: `map["name"]`. |
| Map In-Place Mutation | ✅ | Assignment by key: `map["city"] = "Paris" ;`. |
| 2D Arrays / Nested Lists | ✅ | Arbitrary multi-level list indexing: `matrix[0][1]`. |
| Deep Map/List Traversals | ✅ | Mixed indexing: `data["users"][0]["name"]`. |
| Nested Index Assignment | ✅ | Deep in-place mutation: `data["users"][0]["score"] = 95 ;`. |
| Collection Arguments | ✅ | Pass lists and maps into functions and methods. |
| Collection Return Values | ✅ | Return lists and maps from functions and methods. |

---

## 3. Object-Oriented Programming (OOP)

| Feature | Status | Notes |
|---|:---:|---|
| `class` Declaration | ✅ | Declares classes with fields, constructors, and instance methods. |
| Single Inheritance (`extends`) | ✅ | Subclasses inherit all fields and methods from a declared superclass. |
| Superclass Prior Declaration | ✅ | Parent class must be declared before child class. |
| Duplicate Field Collision Rejection | ✅ | Subclasses declaring identical field names as parent classes are rejected. |
| Self-Inheritance Rejection | ✅ | Circular or self-referential inheritance (`class A extends A`) is rejected. |
| Missing Parent Validation | ✅ | Inheriting from an undeclared parent class produces an error. |
| Instance Fields | ✅ | Typed or untyped instance fields initialized per object. |
| Access Modifiers (`public`/`private`) | ✅ | Supported on fields and methods; defaults to `public`. |
| Encapsulation Enforcement | ✅ | Private members restricted to declaring class methods (`getThisExecClass()`). |
| Subclass Private Access Guard | ✅ | Subclasses cannot access private members of parent classes. |
| Instantiation (`new`) | ✅ | Instantiates objects in expressions, statements, arguments, and collections. |
| Explicit Constructors (`construct`) | ✅ | `construct ( ... )` initializes instances with arguments upon `new`. |
| Implicit Default Constructor | ✅ | Zero-argument construction provided if no constructor is declared. |
| Dynamic Receiver Binding (`this`) | ✅ | Binds current object to `this` inside methods and constructors. |
| Receiver Stack (`thisObjStack`) | ✅ | Dedicated receiver stack prevents receiver leakage across calls. |
| Receiver Call Frame Isolation | ✅ | Nested method calls and free functions (`execute_func_call`) maintain isolated receivers. |
| `this` Field Access & Mutation | ✅ | `this . field` and `this . field = expr ;`. |
| `this` Method Calls | ✅ | `this . otherMethod ( ... )`. |
| Standalone `this` Reference | ✅ | Returning `this` (`return this ;`) or passing `this` to functions. |
| Dynamic Virtual Dispatch | ✅ | Method calls dynamically resolve from receiver's actual runtime class. |
| Method Overriding | ✅ | Child class overrides parent method; runtime dispatch invokes child implementation. |
| Inherited Method Execution | ✅ | Unoverridden parent methods are callable on child instances. |
| Runtime Type Check (`is_instance`) | ✅ | Built-in testing exact class match or any ancestor class in hierarchy. |
| Formatted Object Output | ✅ | Standard representation: `<Object ClassName#id>` (`"null"` if null). |
| Reference Semantics & Aliasing | ✅ | Object variables hold heap references; mutations reflect across aliases. |

---

## 4. Built-in Functions

| Built-in | Syntax | Return Type | Description |
|---|---|:---:|---|
| `len` | `len(coll)` | Number | Returns element count of list or map. |
| `push` | `push(list, val)` | Number | Appends `val` to end of `list`. |
| `pop` | `pop(list)` | Dynamic | Removes and returns last element of `list`. |
| `exists` | `exists(map, key)` | Number (`0`/`1`) | Checks if `key` is present in `map`. |
| `delete` | `delete(map, key)` | Number | Removes key-value pair from `map`. |
| `is_instance` | `is_instance(obj, "Class")` | Number (`0`/`1`) | Checks if `obj` is an instance of `"Class"` or subclass. |
| `out` | `out expr ;` | N/A | Statement outputting expression result with newline. |
| `in` | `in varName ;` | N/A | Statement reading standard input line into variable. |

---

## 5. Runtime Architecture Components

| Component | Module | Implementation Design |
|---|---|---|
| Scope Stack | [`memory.bykig`](memory.bykig) | `scopeStack[1000]`; manages variable indices across nested blocks. |
| Variable Registry | [`memory.bykig`](memory.bykig) | Parallel arrays: `varName`, `varType`, `varValNum`, `varValStr`. |
| Dynamic List Heap | [`memory.bykig`](memory.bykig) | Heap-allocated 2D dynamic arrays with capacity expansion. |
| Generic Map Heap | [`memory.bykig`](memory.bykig) | Key lists and parallel value tables per heap map ID. |
| Object Instance Heap | [`memory.bykig`](memory.bykig) | Parallel field value maps indexed by `[objId][fieldName]`. |
| Class Registry | [`memory.bykig`](memory.bykig) | Metadata maps for classes, inheritance, fields, and methods. |
| Receiver Stack | [`memory.bykig`](memory.bykig) | `thisObjStack` and `thisExecClassStack` for reentrant method dispatch. |
| Expression Engine | [`evaluator.bykig`](evaluator.bykig) | Delimiter-bounded infix evaluation with operator precedence. |
| Member Dot Resolver | [`evaluator.bykig`](evaluator.bykig) | Chained member navigation, method calls, and permission checks. |
| Nested Index Resolver | [`evaluator.bykig`](evaluator.bykig) | Multi-level collection index resolution across lists and maps. |
| Statement Dispatcher | [`main.bykig`](main.bykig) | Token scanner loop, control flow, block skipping, and I/O. |

---

## 6. Implementation Limits & Boundaries

| Boundary / Limitation | Limit / Status | Notes |
|---|---|---|
| Max Fields Per Class | 64 | Enforced during class field registration. |
| Max Methods Per Class | 64 | Enforced during class method registration. |
| Inheritance Model | Single Inheritance | Classes can only inherit from at most one direct superclass (`extends`). Multiple inheritance is not supported. |
| Super Invocation (`super`) | Not supported | Subclass overrides cannot delegate to parent implementation via `super`. |
| Interfaces & Abstract Classes | Not supported | All classes are concrete. |
| Multiple Scope Stack Depth | 1,000 | Depth limit for nested blocks and recursive call scopes. |
| Host Compatibility | Multi-line Blocks | Control-flow blocks in `.bykig` code must strictly format braces on separate lines. |

