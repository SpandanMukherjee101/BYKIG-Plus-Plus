# BYKIG++ Feature Matrix

> Status reflects the current BYKIG++ source tree in `BYKIG++(2).zip`.

## Core language features

| Feature | Status | Notes |
|---|:---:|---|
| `int` | ✅ | Stored as `int`; exposed to the expression engine through `Value`. |
| `float` | ✅ | Native floating-point runtime type. |
| `char` | ✅ | Stored as `char`; expressions expose it numerically. |
| `bool` | ✅ | Stored as `int`; truth values are represented numerically. |
| `string` | ✅ | Fixed-size character buffer per variable. |
| Variable declaration | ✅ | Supported in the active environment. |
| Variable assignment | ✅ | RHS is evaluated through the expression engine. |
| Expression evaluation | ✅ | Arithmetic, comparison, logical operators, parentheses, and complex resolution. |
| `if` / `elif` / `else` | ✅ | Source-position/block based control flow. |
| `while` | ✅ | Loop position is retained so the body can be re-entered. |
| `break` | ✅ | Breaks the active loop. |
| `continue` | ✅ | Skips the current iteration and resumes loop control. |
| `func` | ✅ | Functions are registered by name, parameter metadata, source position, and file path. |
| `return` | ✅ | Produces a runtime `Value` for the active call. |
| Recursion | ✅ | Implemented through the call stack and per-call environments. |
| Module import with `use` | ✅ | Executes another `.bykig` file in the current runtime environment. |
| Comments | ✅ | `//` single-line comments. |
| Program terminator | ✅ | A final `.` is required. |

## Runtime data structures

| Runtime component | Status | Implementation idea |
|---|:---:|---|
| Unified `Value` | ✅ | `ValueType`, scalar storage, string storage, or `void *ptr` for complex objects. |
| Environment stack | ✅ | `envStack[10000]`; active scope is selected with `envTop`. |
| Call stack | ✅ | `callStack[10000]`; active call is selected with `callStackTop`. |
| Function registry | ✅ | Linked list of `funcDef` structures. |
| Typed primitive storage | ✅ | Separate linked lists for `int`, `float`, `char`, `bool`, `string`. |
| Typed arrays | ✅ | Separate structures for int/float/char/bool arrays. |
| Dynamic lists | ✅ | Size/capacity based numeric list. |
| Maps | ✅ | Up to 100 string-key/float-value entries per map. |
| Environment cleanup | ✅ | `cleanupFrame()` frees variables, arrays, list buffers, and maps in the frame. |

## Collections

| Feature | Status | Notes |
|---|:---:|---|
| `intarr` | ✅ | Fixed-size integer array. |
| `floatarr` | ✅ | Fixed-size float array. |
| `chararr` | ✅ | Fixed-size character array. |
| `boolarr` | ✅ | Fixed-size boolean/integer array. |
| `list` | ✅ | Dynamic numeric collection backed by `float *data`. |
| `map` | ✅ | String keys and float values; linear lookup. |
| `arr[index]` | ✅ | Typed array access. |
| `list[index]` | ✅ | Numeric indexed access. |
| `map[key]` | ✅ | Key-based lookup/update. |
| `push()` | ✅ | Adds to a list/map through the built-in layer. |
| `pop()` | ✅ | Removes from a list/map through the built-in layer. |

## Built-ins

| Built-in | Status | Purpose |
|---|:---:|---|
| `len()` | ✅ | Length of supported strings/arrays/lists/maps. |
| `push()` | ✅ | Collection mutation. |
| `pop()` | ✅ | Collection mutation and value production. |
| `exists()` | ✅ | Scope/map existence check. |
| `delete()` | ✅ | Removes supported runtime data. |
| `parse_float()` | ✅ | Converts a string/numeric value to a float result. |
| `file_exists()` | ✅ | Tests whether a file can be opened for reading. |
| `file_write()` | ✅ | Overwrites a file. |
| `file_append()` | ✅ | Appends to a file. |
| `file_read()` | ✅ | Reads a file into the built-in string-return path. |
| `file_scan_open()` | ✅ | Opens persistent scanner state. |
| `file_scan_next()` | ✅ | Reads the next whitespace-delimited token. |
| `file_scan_close()` | ✅ | Closes scanner state. |
| `file_scan_get_pos()` | ✅ | Returns scanner file position. |
| `file_scan_set_pos()` | ✅ | Restores scanner file position. |

## I/O and modules

| Feature | Status | Notes |
|---|:---:|---|
| Console output `out` | ✅ | Supports strings and formatting sequences. |
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
| Expression representation | No AST; expressions are resolved and evaluated directly. |
| Parser/lexer | No traditional compiler-style parser or lexer. |
| Lists | Numeric (`float`) storage rather than arbitrary `Value` elements. |
| Maps | String keys + float values; fixed table of 100 entries; linear search. |
| Complex function results in arithmetic | Current call-result rewriting is primarily numeric (`retVal.f`). |
| Modules | Execute into the current environment rather than a separate namespace. |
| Module cache/cycle system | No dedicated import cache or namespace graph. |
| Function registry lifetime | Registry nodes are process-lifetime allocations in the current source. |
