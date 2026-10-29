# mini-bykig++

**mini-bykig++** is a fully-featured, self-hosting-ready interpreter for the `.mini` programming language, written entirely in **BYKIG++**. It runs on top of the native BYKIG++ runtime (`Interpreter.exe`) and provides a modern, expressive dynamically-typed scripting environment with dynamic collections, generic associative maps, nested indexing, functions, and lexical scoping.

---

## Architecture

The interpreter is cleanly modularized into three core BYKIG++ modules:

| Module | Purpose |
|---|---|
| [`memory.bykig`](file:///c:/Users/spand/OneDrive/Documents/MyFiles/BYKIG++/mini-bykig++/memory.bykig) | **Scoped Lexical Environment & Heap Storage**: Manages lexical block scopes (`pushScope`, `popScope`), variable resolution, variable assignment, shadowing, heap-allocated dynamic lists (`newList`, `listPush`, `listPop`, `listSet`, `listGetVal`), generic associative maps (`newMap`, `mapSet`, `mapGetVal`, `mapExists`, `mapDelete`), recursive `Value` formatting (`formatValue`), and the user function registry. |
| [`evaluator.bykig`](file:///c:/Users/spand/OneDrive/Documents/MyFiles/BYKIG++/mini-bykig++/evaluator.bykig) | **Expression, Collection & Call Evaluator**: Evaluates multi-term infix arithmetic, boolean logic, string concatenation, collection literals (`[ ... ]` and `{ ... }`), multi-level nested index resolution (`resolve_indexed_val`), built-in collection operations (`len`, `push`, `pop`, `exists`, `delete`), function argument lists, and user function execution. |
| [`main.bykig`](file:///c:/Users/spand/OneDrive/Documents/MyFiles/BYKIG++/mini-bykig++/main.bykig) | **Execution Loop & CLI**: Interactive CLI entry point, statement dispatch, block execution, control flow handling (`if`, `elif`, `else`, `while`, `for`, `break`, `continue`), nested index assignment (`execute_index_assignment`), standalone built-in execution, and I/O. |

---

## Language Features (`.mini`)

### 1. Variables & Types
- **Numbers**: Integers and floating-point numbers.
- **Strings**: Quoted string literals (supports spaces and concatenation with `+`).
- **Booleans**: `true` and `false` (resolved to `1` and `0` in logical operations).
- **Dynamic Lists**: Heap-managed arrays containing numbers, strings, lists, or maps.
- **Generic Maps**: Key-value associative tables with string or numeric keys.
- **Declaration & Assignment**:
  ```mini
  var x = 42 ;
  var greeting = "Hello, world!" ;
  x = x + 10 ;
  ```

### 2. Dynamic Lists
- **Literals**: List literals use bracket notation `[ elem1 , elem2 , ... ]`:
  ```mini
  var lst = [ 10 , 20 , 30 ] ;
  out lst ; // Prints [10, 20, 30]
  ```
- **Indexing & Mutation**:
  ```mini
  out lst[0] ; // 10
  lst[1] = 99 ;
  out lst[1] ; // 99
  ```
- **Built-in Operations**:
  - `push(lst, val)`: Appends an element to the list.
  - `pop(lst)`: Removes and returns the last element.
  - `len(lst)`: Returns the current element count.

### 3. Generic Associative Maps
- **Literals**: Key-value map literals use brace notation `{ "key" : val , ... }`:
  ```mini
  var person = { "name" : "Alice" , "age" : 30 } ;
  out person ; // Prints {"name": Alice, "age": 30}
  ```
- **Key Lookup & Update**:
  ```mini
  out person["name"] ; // Alice
  person["city"] = "Paris" ;
  out person["city"] ; // Paris
  ```
- **Built-in Operations**:
  - `exists(map, key)`: Returns `1` if key is present, `0` otherwise.
  - `delete(map, key)`: Removes a key-value entry from the map.
  - `len(map)`: Returns the number of entries in the map.

### 4. Nested List & Map Indexing
- Deep read traversal and index assignment across multi-dimensional collections:
  ```mini
  // 2D Array / Nested List
  var matrix = [ [ 1 , 2 ] , [ 3 , 4 ] ] ;
  out matrix[0][0] ; // 1
  matrix[1][0] = 77 ;
  out matrix ; // [[1, 2], [77, 4]]

  // Complex Nested Maps & Lists
  var data = { "users" : [ { "name" : "Bob" , "score" : 50 } ] } ;
  out data["users"][0]["name"] ; // Bob
  data["users"][0]["score"] = 95 ;
  out data["users"][0]["score"] ; // 95
  ```

### 5. Lexical Scoping & Shadowing
- Each block `{ ... }` introduces an isolated scope level.
- Inner variables cleanly shadow outer variables without overwriting the outer scope:
  ```mini
  var x = 100 ;
  if x == 100
  {
      var x = 200 ;
      out x ; // Prints 200
  }
  out x ; // Prints 100
  ```

### 6. Expressions & Operators
- **Arithmetic**: `+`, `-`, `*`, `/`, `%` with standard operator precedence and grouping `( ... )`.
- **Relational**: `==`, `!=`, `<`, `>`, `<=`, `>=`.
- **Logical**: `&&`, `||`, `!`.
- **String Concatenation**: `+` handles strings, variables, and numbers seamlessly.

### 7. Control Flow
- **Conditional branching**:
  ```mini
  if score >= 90
  {
      out "Grade: A" ;
  }
  elif score >= 80
  {
      out "Grade: B" ;
  }
  else
  {
      out "Grade: C" ;
  }
  ```
- **While Loops** (with `break` and `continue` support):
  ```mini
  var i = 0 ;
  while i < 10
  {
      i = i + 1 ;
      if i == 3 { continue ; }
      if i > 5 { break ; }
      out i ;
  }
  ```
- **For Loops**:
  ```mini
  for var i = 1 ; i <= 5 ; i = i + 1
  {
      out i ;
  }
  ```

### 8. Functions & Recursion
- User-defined functions support multiple arguments and return statements.
- Procedures (functions with side-effects and no explicit return) are supported.
- Collections can be passed as arguments and returned from functions:
  ```mini
  func getFirst ( list )
  {
      return list[0] ;
  }

  var first = getFirst ( [ 100 , 200 ] ) ;
  out first ; // 100
  ```
- Supports recursive calls (e.g., Fibonacci, Factorial) and nested function calls inside expressions:
  ```mini
  func fact ( n )
  {
      if n <= 1
      {
          return 1 ;
      }
      return n * fact ( n - 1 ) ;
  }

  var res = fact ( 5 ) ;
  out res ; // 120
  ```

### 9. I/O & Comments
- `out <expr> ;`: Prints numbers, strings, collections, or expression results followed by a newline.
- `in <var> ;`: Reads user input from standard input.
- `//`: Single-line comments anywhere in source files are ignored by the scanner.

---

## Test Suites

The test suite is organized into focused `.mini` programs:

1. [`test_basic.mini`](file:///c:/Users/spand/OneDrive/Documents/MyFiles/BYKIG++/mini-bykig++/test_basic.mini): Arithmetic precedence, string concatenation, variable mutation, boolean comparisons.
2. [`test_control.mini`](file:///c:/Users/spand/OneDrive/Documents/MyFiles/BYKIG++/mini-bykig++/test_control.mini): `if`/`elif`/`else`, `while` with `break` & `continue`, `for` loop, block-level variable shadowing.
3. [`test_func.mini`](file:///c:/Users/spand/OneDrive/Documents/MyFiles/BYKIG++/mini-bykig++/test_func.mini): Multi-parameter functions, string return values, procedures, recursion (`factorial`), and chained calls in expressions.
4. [`test_collections.mini`](file:///c:/Users/spand/OneDrive/Documents/MyFiles/BYKIG++/mini-bykig++/test_collections.mini): Dynamic lists, generic maps, `push`, `pop`, `len`, `exists`, `delete`, multi-dimensional list indexing (`matrix[1][0]`), deep map/list nesting (`data["users"][0]["name"]`), and collection parameters in functions.
5. [`test_all.mini`](file:///c:/Users/spand/OneDrive/Documents/MyFiles/BYKIG++/mini-bykig++/test_all.mini): Comprehensive end-to-end regression covering all language features including double recursion (`fibonacci`).

---

## Running mini-bykig++

To run any `.mini` file through the `mini-bykig++` interpreter:

```powershell
echo mini-bykig++/test_collections.mini | .\Interpreter.exe mini-bykig++/main.bykig
```

Or run the full test suite:
```powershell
echo mini-bykig++/test_all.mini | .\Interpreter.exe mini-bykig++/main.bykig
```

Or interactively:
```powershell
.\Interpreter.exe mini-bykig++/main.bykig
# When prompted, type: mini-bykig++/test_all.mini
```

