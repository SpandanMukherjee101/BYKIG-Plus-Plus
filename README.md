# BYKIG++

**BYKIG++** is a lightweight, dynamically interpreted programming language implemented in C.

It extends the original BYKIG interpreter with a broader runtime model while retaining the project's compact, data-structure-driven architecture: linked lists, stacks, environments, and direct source-position execution instead of a traditional compiler pipeline.

## Highlights

- Primitive types: `int`, `float`, `char`, `bool`, `string`
- Arithmetic, comparison, and logical expressions
- `if`, `elif`, `else`, `while`, `break`, `continue`
- Functions, return values, nested calls, and recursion
- Function-local environments and scope-aware lookup
- Primitive arrays
- Dynamic numeric lists
- String-key/float-value maps
- Built-in collection and filesystem APIs
- Streaming file scanner
- Multi-file execution through `use`
- Runtime `Value` abstraction
- Frame cleanup for function environments

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
             ┌──────────────┼──────────────┐
             ▼              ▼              ▼
          envStack       callStack         FN
             │              │              │
             ▼              ▼              ▼
        variables       invocations      functions
        arrays
        lists
        maps
```

BYKIG++ does not build a traditional AST. Functions and control flow rely heavily on source positions (`ftell`/`fseek`) and interpreter state.

## Documentation

| Document | Purpose |
|---|---|
| [`FeatureMatrix.md`](FeatureMatrix.md) | Supported features, runtime structures, built-ins, and current implementation boundaries. |
| [`UserGuide.md`](UserGuide.md) | Syntax, examples, language usage, CLI usage, I/O, collections, functions, and modules. |
| [`DevGuide.md`](DevGuide.md) | Runtime architecture, source layout, extension points, memory ownership, functions, expressions, and modules. |

## Quick example

```bykig
func add ( int a , int b )
{
    return a + b ;
}

int result = add ( 5 , 10 ) ;
out "Result: " + result + "%n" ;
.
```

## Syntax reminders

BYKIG++ is intentionally strict about spacing. Prefer:

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

## Current implementation notes

- Primitive variables and collections are stored in specialized linked-list structures.
- `Value` provides a common runtime representation for expression results and complex objects.
- Function calls create a new environment and call frame; frame-owned allocations are cleaned when the call unwinds.
- Lists currently store numeric `float` elements.
- Maps currently use fixed arrays of string keys and float values.
- `use` executes another source file in the current runtime environment rather than creating a module namespace.
- The interpreter remains source-position-driven rather than AST-driven.

## Author

Made by **Spandan Mukherjee**.

Originally documented as completed on **06/09/2026**.
