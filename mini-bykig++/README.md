# mini-bykig++

**mini-bykig++** is a fully-featured, self-hosting-ready interpreter for the `.mini` programming language, written entirely in **BYKIG++**.

It runs on top of the native BYKIG++ runtime (`Interpreter.exe`) and provides a modern, expressive dynamically-typed scripting environment featuring dynamic collections, generic associative maps, multi-level nested indexing, lexical scoping, functions, recursion, and a complete Object-Oriented Programming (OOP) system.

---

## Highlights

- **Primitive Types & Dynamic Typing**: Numbers (int/float), strings, booleans (`true`/`false`), and dynamic runtime values.
- **Arithmetic & Logical Expressions**: Multi-term infix arithmetic (`+`, `-`, `*`, `/`, `%`), comparison (`==`, `!=`, `<`, `>`, `<=`, `>=`), and boolean logic (`&&`, `||`, `!`).
- **Control Flow**: Conditional branching (`if`, `elif`, `else`), `while` loops with `break` and `continue`, and three-clause `for` loops.
- **Lexical Scoping & Shadowing**: Block-level variable isolation (`pushScope` / `popScope`) and automatic variable shadowing.
- **Functions & Recursion**: First-class procedures, multi-parameter functions, return statements, and recursion (single & double recursion).
- **Dynamic Heap Lists**: Resizable heterogeneous lists (`[ ... ]`) with in-place index mutation, `push`, `pop`, and `len`.
- **Generic Associative Maps**: Hash maps (`{ ... }`) with string/numeric keys, in-place key updates, `exists`, `delete`, and `len`.
- **Multi-Level Nested Indexing**: Arbitrary deep read traversal and index assignment across multi-dimensional collections (`matrix[0][1] = 99 ;`, `data["users"][0]["name"]`).
- **Object-Oriented Programming**:
  - Class declarations with fields, explicit constructors (`construct`), and instance methods.
  - Implicit zero-argument default constructors when omitted.
  - Explicit parent constructor chaining via `super . construct ( ... )` with first-statement, single-execution, and superclass invariants.
  - Implicit ancestor constructor chaining when `super.construct` is omitted (with diagnostics if ancestor constructors require arguments).
  - Instantiation via `new ClassName ( ... )` in expressions, statements, collections, and arguments.
  - Dynamic receiver binding (`this`) with call frame isolation across nested/recursive calls.
  - Member access and mutation via standard dot notation (`obj . field = ...`, `obj . method ( ... )`).
  - Single inheritance (`class Dog extends Animal`), inherited member access, circular inheritance rejection, and duplicate field collision guards.
  - Virtual method overriding and dynamic runtime polymorphic dispatch.
  - Access control and encapsulation with `public` and `private` modifiers enforced against declaring class context.
  - Null reference safety: uninitialized typed reference declarations (`Point p ;`), explicit `null` literal, `is_null ( expr )` predicate, and clear diagnostics on null dereference or null method call.
  - Runtime object type inspection predicate: `is_instance ( obj , "ClassName" )`.
  - Standard object string representation: `<Object ClassName#id>` (`"null"` for null references).
- **100% Pure BYKIG++**: Implemented entirely in BYKIG++ across three decoupled modules without external runtime dependencies.

---

## Architecture at a Glance

```text
                     .mini Source Code
                            │
                            ▼
                  ┌───────────────────┐
                  │    main.bykig     │  Token Scanner & CLI Loop
                  │  Statement Engine │  Control Flow (if, while, for)
                  └─────────┬─────────┘
                            │
         ┌──────────────────┴──────────────────┐
         ▼                                     ▼
┌───────────────────┐                 ┌───────────────────┐
│  evaluator.bykig  │                 │   memory.bykig    │
│  Infix Evaluator  │                 │  Lexical Scopes   │
│ Collection Parser │◄───────────────►│ List & Map Heaps  │
│    OOP Dispatch   │                 │ Class & Heap Reg  │
│  Member Dot Access│                 │ Receiver Stack    │
└───────────────────┘                 └───────────────────┘
```

### Module Responsibilities

| Module | File | Purpose |
|---|---|---|
| **Memory & Heap** | [`memory.bykig`](memory.bykig) | **Scoped Lexical Environment, Heap Storage & OOP Registry**: Manages lexical block scopes (`pushScope`, `popScope`), variable resolution, variable assignment, shadowing, heap-allocated dynamic lists, generic associative maps, class registry (`registerClass`, `registerClassField`, `registerClassMethod`, `registerClassConstructor`), instance storage (`newObjectInstance`, `setObjectField`, `getObjectFieldVal`), inheritance lookup (`findMethodClass`, `isSubclass`), member encapsulation checks (`canAccessField`), receiver stack (`pushThis`, `popThis`, `getThisObj`), and recursive `Value` formatting (`formatValue`) supporting `<Object ClassName#id>`. |
| **Evaluator** | [`evaluator.bykig`](evaluator.bykig) | **Expression, Collection, OOP & Call Evaluator**: Evaluates multi-term infix arithmetic, boolean logic, string concatenation, collection literals (`[ ... ]` and `{ ... }`), multi-level nested index resolution (`resolve_indexed_val`), built-in collection operations (`len`, `push`, `pop`, `exists`, `delete`, `is_instance`), constructor execution (`execute_class_constructor`), method invocation (`execute_method_call`), chained member dot resolution (`resolve_member_access`), instantiation (`new ClassName ( ... )`), receiver binding (`this`), function argument lists, and user function execution. |
| **Execution Loop** | [`main.bykig`](main.bykig) | **Execution Loop & CLI**: Interactive CLI entry point, statement dispatch, block execution, control flow handling (`if`, `elif`, `else`, `while`, `for`, `break`, `continue`), class definition parsing (`class <Name> [extends <Parent>]`), member assignment statements (`obj . field = ...`, `this . field = ...`), standalone method calls, typed variable declarations, nested index assignment (`execute_index_assignment`), standalone built-in execution, and I/O. |

---

## Documentation

| Document | Purpose |
|---|---|
| [`UserGuide.md`](UserGuide.md) | Syntax rules, types, expressions, control flow, functions, dynamic collections, nested indexing, complete OOP guide, built-in reference, and execution. |
| [`DevGuide.md`](DevGuide.md) | Implementation architecture, memory models, heap representations, class registry, receiver isolation, expression evaluation, statement dispatch, and host runtime invariants. |
| [`FeatureMatrix.md`](FeatureMatrix.md) | Complete support matrix for core language features, collections, OOP capabilities, built-in functions, and current system boundaries. |

---

## Quick Example

```mini
class Animal
{
    public string name ;

    public func construct ( petName )
    {
        this . name = petName ;
    }

    public func speak ( )
    {
        out this . name + " makes a sound." ;
    }
}

class Dog extends Animal
{
    public string breed ;

    public func construct ( petName , petBreed )
    {
        super . construct ( petName ) ;
        this . breed = petBreed ;
    }

    public func speak ( )
    {
        out this . name + " barks! (" + this . breed + ")" ;
    }
}

// Polymorphic invocation
func announce ( pet )
{
    pet . speak ( ) ;
}

var myDog = new Dog ( "Buddy" , "Golden Retriever" ) ;
announce ( myDog ) ; // Buddy barks! (Golden Retriever)

// Dynamic collections & built-ins
var pets = [ myDog ] ;
push ( pets , new Animal ( "Creature" ) ) ;
out len ( pets ) ; // 2

// Runtime type verification & null checks
out is_instance ( myDog , "Animal" ) ; // 1
out is_null ( myDog ) ; // 0
Dog uninitDog ;
out is_null ( uninitDog ) ; // 1
```

---

## Test Suites

The test suite is organized into focused `.mini` programs located in [`tests/`](tests/):

1. [`test_basic.mini`](tests/test_basic.mini): Arithmetic precedence, string concatenation, variable mutation, boolean comparisons.
2. [`test_control.mini`](tests/test_control.mini): `if`/`elif`/`else`, `while` with `break` & `continue`, `for` loop, block-level variable shadowing.
3. [`test_func.mini`](tests/test_func.mini): Multi-parameter functions, string return values, procedures, recursion (`factorial`), and chained calls in expressions.
4. [`test_collections.mini`](tests/test_collections.mini): Dynamic lists, generic maps, `push`, `pop`, `len`, `exists`, `delete`, multi-dimensional list indexing (`matrix[1][0]`), deep map/list nesting (`data["users"][0]["name"]`), and collection parameters in functions.
5. [`test_oop.mini`](tests/test_oop.mini): Classes, fields, constructors, `this` receiver binding & mutation, single and multi-level inheritance, method overriding, runtime polymorphic dispatch, `super . construct ( ... )` parent constructor chaining, implicit ancestor construction, encapsulation violations, uninitialized/null references, null member diagnostics, and `is_instance` / `is_null` runtime predicates.
6. [`test_all.mini`](tests/test_all.mini): Comprehensive end-to-end regression covering all language features including double recursion (`fibonacci`) and full OOP integration with `super . construct ( ... )`.

---

## Running mini-bykig++

To run any `.mini` file through the `mini-bykig++` interpreter:

```powershell
echo mini-bykig++/tests/test_collections.mini | .\Interpreter.exe mini-bykig++/main.bykig
```

Or run the full test suite:

```powershell
echo mini-bykig++/tests/test_all.mini | .\Interpreter.exe mini-bykig++/main.bykig
```

Or interactively:

```powershell
.\Interpreter.exe mini-bykig++/main.bykig
# When prompted, type: mini-bykig++/tests/test_all.mini
```
