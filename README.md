# BYKIG++

**BYKIG++** is a lightweight, dynamically interpreted programming language implemented in C.

It extends the original BYKIG interpreter with a broader runtime model while retaining the project's compact, data-structure-driven architecture: linked lists, stacks, environments, and direct source-position execution instead of a traditional compiler pipeline.

## Highlights

- Primitive types: `int`, `float`, `char`, `bool`, `string`
- Arithmetic, comparison, and logical expressions
- Control flow: `if`, `elif`, `else`, `while`, `for`, `break`, `continue`
- Functions, return values, nested calls, and recursion
- Function-local environments and scope-aware lookup with variable shadowing
- Primitive arrays: `intarr`, `floatarr`, `chararr`, `boolarr`
- Dynamic generic lists with arbitrary `Value` elements and capacity growth
- Generic dynamic associative maps with `Value` values and dynamic capacity
- Multi-dimensional and nested indexing (`matrix[0][1] = 99 ;`, `data["users"][0]["name"]`)
- Object-Oriented Programming: classes, typed fields, explicit constructors, instance methods, and `this` receiver binding
- Encapsulation with `public` and `private` member access modifiers
- Single inheritance (`extends`), dynamic virtual dispatch, and runtime polymorphism
- Runtime object type predicate: `is_instance(obj, "ClassName")`
- Centralized runtime heap (`OBJECT_HEAP`) with safe reference semantics and deterministic shutdown cleanup
- Built-in collection operations: `len`, `push`, `pop`, `exists`, `delete`
- Expanded utility built-ins: `quote`, `strip_quotes`, `is_num`, `is_str`, `starts_with`, `parse_float`
- Streaming file scanner (`file_scan_*`) with punctuation-aware tokenization
- Multi-file execution through `use`
- Unified runtime `Value` abstraction
- Frame cleanup for scoped environments and function calls
- Includes [`mini-bykig++`](mini-bykig++/README.md), a self-hosting-ready companion interpreter written entirely in BYKIG++

## Architecture at a glance

```text
                    BYKIG++ Source
                           │
                           ▼
                 run_interpreter_loop()
                     /          \
                    /            \
             statements       expressions
                  │                 │
                  │          resolve_complex_types()
                  │                 │
                  │                 ▼
                  │               val()
                  │                 │
                  │                 ▼
                  │              Value
                  │                 │
                  └──────────┬──────┘
                             ▼
                      Runtime state
             ┌──────────────┼──────────────┬──────────────┬──────────────┐
             ▼              ▼              ▼              ▼              ▼
          envStack       callStack         FN       CLASS_REGISTRY  OBJECT_HEAP
             │              │              │              │              │
             ▼              ▼              ▼              ▼              ▼
        variables       invocations    functions       classes        objects
        arrays          (thisObj)
        lists           (executingClass)
        maps
        objects
```

BYKIG++ does not build a traditional AST. Functions and control flow rely heavily on source positions (`ftell`/`fseek`) and interpreter state.

## Documentation

| Document | Purpose |
|---|---|
| [`FeatureMatrix.md`](FeatureMatrix.md) | Supported features, runtime structures, built-ins, and current implementation boundaries. |
| [`UserGuide.md`](UserGuide.md) | Syntax, examples, language usage, CLI usage, I/O, collections, functions, and modules. |
| [`DevGuide.md`](DevGuide.md) | Runtime architecture, source layout, extension points, memory ownership, functions, expressions, and modules. |
| [`mini-bykig++/`](mini-bykig++/README.md) | Self-hosted interpreter written in BYKIG++ with its own documentation suite ([README](mini-bykig++/README.md), [User Guide](mini-bykig++/UserGuide.md), [Dev Guide](mini-bykig++/DevGuide.md), [Feature Matrix](mini-bykig++/FeatureMatrix.md)). |

## Quick example

```bykig
func add ( int a , int b )
{
    return a + b ;
}

int result = add ( 5 , 10 ) ;
out "Result: " + result + "%n" ;

list numbers = { 10 , 20 , 30 } ;
push ( numbers , 40 ) ;
out "List: " + numbers + "%n" ;

map user = { "name" : "Alice" , "role" : "admin" } ;
user [ "city" ] = "Paris" ;
out "User: " + user + "%n" ;

class Animal
{
    public string name ;
    public func speak ( )
    {
        return "Generic sound" ;
    }
}

class Dog extends Animal
{
    public func construct ( string n )
    {
        this . name = n ;
    }

    public func speak ( )
    {
        return "Woof!" ;
    }
}

Animal pet = new Dog ( "Buddy" ) ;
out pet . name + " says: " + pet . speak ( ) + "%n" ;
out "Is Dog? " + is_instance ( pet , "Dog" ) + "%n" ;
.
```

## Syntax reminders

BYKIG++ uses clean token spacing. Prefer:

```bykig
int x = 5 ;
```

rather than:

```bykig
int x=5;
```

For `if`, `elif`, `else`, and `while`, put `{` on the following line:

```bykig
if x > 5
{
    out "yes%n" ;
}
```

End a complete program with `.`.

## Running

Windows PowerShell:

```powershell
.\Interpreter.exe path\to\program.bykig
```

Running without a file argument starts the filename-prompt mode.

To run the `mini-bykig++` interpreter on a `.mini` program:

```powershell
echo mini-bykig++/test_collections.mini | .\Interpreter.exe mini-bykig++/main.bykig
```

## Current implementation notes

- Primitive variables and collections are stored in specialized linked-list structures.
- `Value` provides a common runtime representation for expression results and complex objects.
- Function calls create a new environment and call frame; frame-owned allocations are cleaned when the call unwinds.
- Dynamic lists store generic `Value` elements (numbers, strings, nested lists, maps).
- Dynamic maps use generic `Value`-backed dynamic entries with automatic capacity growth (`realloc`).
- Nested indexing is fully supported for expressions and assignments.
- Object instances are heap-allocated (`OBJECT_HEAP`) with reference semantics, managed by deterministic shutdown reclamation.
- Classes support single inheritance (`extends`), virtual method overriding, explicit `construct` constructors, and encapsulation (`public`/`private`).
- Runtime object type predicates are provided via `is_instance(obj, "ClassName")`.
- `use` executes another source file in the current runtime environment rather than creating an isolated module namespace.
- The interpreter remains source-position-driven rather than AST-driven.

## Author

Made by **Spandan Mukherjee**.

Originally documented as completed on **29/10/2026**.
