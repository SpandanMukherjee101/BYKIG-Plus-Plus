# mini-bykig++ User Guide

**mini-bykig++** is a lightweight, dynamically-typed scripting language implemented entirely in BYKIG++. It runs on top of the native BYKIG++ runtime (`Interpreter.exe`) and provides a modern language environment featuring dynamic collections, generic associative maps, multi-level nested indexing, lexical scoping, functions, recursion, and a complete Object-Oriented Programming (OOP) system.

---

## 1. Syntax Rules

### Token Spacing
Like BYKIG++, `.mini` programs require clean token spacing. All tokens, operators, delimiters, and semicolons must be delimited by spaces:

```mini
// Correct
var x = 5 ;
var total = ( x + 10 ) * 2 ;

// Incorrect
var x=5;
```

### Statements & Semicolons
Statements must conclude with a spaced semicolon `;`:

```mini
out "Hello, World!" ;
```

### Control Flow Block Formatting
For control blocks (`if`, `elif`, `else`, `while`, `for`, and `func`), the opening brace `{` should be placed on its own line:

```mini
if score >= 90
{
    out "Grade: A" ;
}
else
{
    out "Grade: B" ;
}
```

### Comments
Single-line comments begin with `//` and continue to the end of the line. Comments are ignored by the interpreter:

```mini
// This is a comment
var count = 0 ; // Counter initialization
```

---

## 2. Variables & Data Types

Variables in `.mini` are dynamically typed and can be declared using `var` or explicit primitive type annotations (`int`, `float`, `string`, `bool`, or a registered class name).

### Primitive Types
- **Numbers**: Integers and floating-point values (e.g., `42`, `3.14`).
- **Strings**: Quoted text literals (e.g., `"Hello"`, `"Value: 100"`). Supports spaces and punctuation.
- **Booleans**: `true` and `false` (evaluate numerically to `1` and `0` in logical operations).

### Declarations & Mutation
```mini
var a = 10 ;
var b = "BYKIG++" ;
var flag = true ;

// Re-assignment
a = a + 5 ;
b = "mini-" + b ;
```

Typed declarations and uninitialized declarations are also supported:
```mini
int score ;
score = 95 ;
string name = "Alice" ;
```

---

## 3. Expressions & Operators

The expression evaluator supports arbitrary multi-term expressions with standard precedence and parentheses grouping.

### Arithmetic Operators
- `+` : Addition and string concatenation
- `-` : Subtraction
- `*` : Multiplication
- `/` : Division
- `%` : Modulo

```mini
var result = ( 10 + 20 ) * 3 - 50 / 2 ;
out result ; // 65
```

### Relational & Comparison Operators
- `==` : Equal to
- `!=` : Not equal to
- `<`  : Less than
- `>`  : Greater than
- `<=` : Less than or equal to
- `>=` : Greater than or equal to

```mini
var isGreater = 10 > 5 ; // 1
```

### Logical Operators
- `&&` : Logical AND
- `||` : Logical OR
- `!`  : Logical NOT

```mini
if x > 0 && y < 10
{
    out "Valid range" ;
}
```

### String Concatenation
The `+` operator automatically coerces numbers, booleans, and variables into strings when concatenated with a string literal:

```mini
var name = "Bob" ;
var age = 25 ;
out name + " is " + age + " years old." ;
```

---

## 4. Control Flow

### Conditional Branching (`if`, `elif`, `else`)
```mini
var score = 85 ;

if score >= 90
{
    out "Grade: A" ;
}
elif score >= 80
{
    out "Grade: B" ;
}
elif score >= 70
{
    out "Grade: C" ;
}
else
{
    out "Grade: F" ;
}
```

### While Loops
`while` loops execute while the condition evaluates to true. `break` immediately terminates the loop, and `continue` skips to the next iteration:

```mini
var i = 0 ;
while i < 10
{
    i = i + 1 ;
    if i == 3
    {
        continue ;
    }
    if i > 7
    {
        break ;
    }
    out i ;
}
```

### For Loops
`for` loops accept an initializer, condition, and step statement:

```mini
for var i = 1 ; i <= 5 ; i = i + 1
{
    out i ;
}
```

### Lexical Scoping & Shadowing
Every code block `{ ... }` creates a new lexical scope. Variables declared within inner blocks cleanly shadow outer variables without overwriting them:

```mini
var val = 100 ;
if val == 100
{
    var val = 999 ; // Shadows outer 'val'
    out val ;       // Prints 999
}
out val ;           // Prints 100
```

---

## 5. Dynamic Lists

Lists are ordered, heap-allocated dynamic collections of heterogeneous values.

### Literals & Indexing
List literals use bracket notation `[ ... ]`. Elements are accessed using zero-based bracket indexing `lst[idx]`:

```mini
var numbers = [ 10 , 20 , 30 ] ;
out numbers[0] ; // 10
out numbers[2] ; // 30
```

### In-Place Mutation
```mini
numbers[1] = 99 ;
out numbers ; // [10, 99, 30]
```

### List Built-in Functions
- `len(lst)`: Returns the number of elements in the list.
- `push(lst, val)`: Appends an element to the end of the list.
- `pop(lst)`: Removes and returns the last element.

```mini
var items = [ ] ;
push ( items , "first" ) ;
push ( items , "second" ) ;
out len ( items ) ; // 2

var last = pop ( items ) ;
out last ;          // second
out len ( items ) ; // 1
```

---

## 6. Generic Associative Maps

Maps are key-value associative tables supporting string or numeric keys and arbitrary values.

### Literals & Lookup
Map literals use brace notation `{ "key" : val , ... }`:

```mini
var user = { "name" : "Alice" , "role" : "admin" , "level" : 5 } ;
out user["name"] ; // Alice
out user["level"] ; // 5
```

### In-Place Insertion & Update
```mini
user["city"] = "Paris" ;
user["level"] = 6 ;
out user ; // {"name": Alice, "role": admin, "level": 6, "city": Paris}
```

### Map Built-in Functions
- `exists(map, key)`: Returns `1` if the key exists, `0` otherwise.
- `delete(map, key)`: Removes the key-value pair from the map.
- `len(map)`: Returns the number of entries in the map.

```mini
out exists ( user , "city" ) ; // 1
delete ( user , "role" ) ;
out exists ( user , "role" ) ; // 0
out len ( user ) ;             // 3
```

---

## 7. Multi-Dimensional & Nested Indexing

`mini-bykig++` supports deep indexing traversal and mutation across arbitrary depths of nested lists and maps:

### Nested Lists (2D Arrays / Matrices)
```mini
var matrix = [ [ 1 , 2 ] , [ 3 , 4 ] ] ;
out matrix[1][0] ; // 3

matrix[1][0] = 77 ;
out matrix[1][0] ; // 77
out matrix ;       // [[1, 2], [77, 4]]
```

### Deeply Nested Maps & Lists
```mini
var response = {
    "status" : 200 ,
    "payload" : [
        { "id" : 101 , "name" : "Server A" } ,
        { "id" : 102 , "name" : "Server B" }
    ]
} ;

out response["payload"][0]["name"] ; // Server A

response["payload"][0]["name"] = "Cluster Master" ;
out response["payload"][0]["name"] ; // Cluster Master
```

---

## 8. Functions & Recursion

User-defined functions support parameters, return values, and recursion.

### Function Declarations & Calls
```mini
func multiply ( a , b )
{
    return a * b ;
}

var product = multiply ( 6 , 7 ) ;
out product ; // 42
```

### Procedures (Void Functions)
Functions without an explicit return statement execute for their side effects:

```mini
func greet ( name )
{
    out "Welcome, " + name + "!" ;
}

greet ( "Alice" ) ;
```

### Passing & Returning Collections
Collections and objects can be passed into functions and returned from them:

```mini
func createPair ( first , second )
{
    return [ first , second ] ;
}

var pair = createPair ( "x" , 100 ) ;
out pair[0] ; // x
out pair[1] ; // 100
```

### Recursion
Functions can call themselves recursively:

```mini
// Factorial
func factorial ( n )
{
    if n <= 1
    {
        return 1 ;
    }
    return n * factorial ( n - 1 ) ;
}

out factorial ( 5 ) ; // 120

// Fibonacci (Double recursion)
func fib ( n )
{
    if n <= 1
    {
        return n ;
    }
    return fib ( n - 1 ) + fib ( n - 2 ) ;
}

out fib ( 7 ) ; // 13
```

---

## 9. Object-Oriented Programming (OOP)

`mini-bykig++` provides a complete class-based object-oriented programming system matching BYKIG++ v8 standards.

### Class Declarations
Classes define member fields, constructors, and instance methods:

```mini
class Point
{
    public float x ;
    public float y ;

    public func construct ( initialX , initialY )
    {
        this . x = initialX ;
        this . y = initialY ;
    }

    public func printCoords ( )
    {
        out "(" + this . x + ", " + this . y + ")" ;
    }
}
```

### Instantiation (`new`)
Use the `new` operator to create an instance of a class:

```mini
var pt = new Point ( 10.5 , 20.0 ) ;
pt . printCoords ( ) ; // (10.5, 20)
```

### Default Zero-Argument Constructors
If a class does not define an explicit constructor, `mini-bykig++` provides an implicit default constructor initializing all declared fields to default values (`0`, `""`, or `null`):

```mini
class Blank
{
    public val ;
}

var b = new Blank ( ) ;
out b . val ; // 0
```

### Receiver Binding (`this`)
Within instance methods and constructors, `this` dynamically refers to the active receiver:
- Reading fields: `this . field`
- Mutating fields: `this . field = value ;`
- Calling sibling methods: `this . otherMethod ( ) ;`
- Returning the receiver: `return this ;`

```mini
class Counter
{
    public count ;

    public func construct ( )
    {
        this . count = 0 ;
    }

    public func increment ( )
    {
        this . count = this . count + 1 ;
        return this ;
    }
}

var c = new Counter ( ) ;
c . increment ( ) ;
out c . count ; // 1
```

### Member Access & Mutation
Access fields and invoke methods using standard dot notation:

```mini
out pt . x ;
pt . x = 99.0 ;
out pt . x ; // 99
```

### Single Inheritance (`extends`)
Subclasses inherit fields and methods from their parent class:

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
```

### Parent Constructor Chaining (`super . construct`)
Derived class constructors can explicitly invoke their parent class constructor using `super . construct ( ... ) ;`:

```mini
class Grandparent
{
    public string gName ;
    public func construct ( g )
    {
        this . gName = g ;
    }
}

class ParentCls extends Grandparent
{
    public string pName ;
    public func construct ( g , p )
    {
        super . construct ( g ) ;
        this . pName = p ;
    }
}

class ChildCls extends ParentCls
{
    public string cName ;
    public func construct ( g , p , c )
    {
        super . construct ( g , p ) ;
        this . cName = c ;
    }
}
```

#### Invariants & Rules for `super . construct`
1. **First-Statement Requirement**: `super . construct ( ... ) ;` must be the very first statement executed inside the derived constructor. Executing other statements beforehand triggers an error.
2. **Single-Invocation Requirement**: `super . construct ( ... ) ;` may be called at most once per constructor execution.
3. **Constructor-Only**: Calling `super . construct` outside a constructor produces a runtime error.
4. **Superclass Requirement**: Calling `super . construct` in a root class with no superclass produces an error.

### Implicit Ancestor Constructor Chaining
If a derived class constructor omits `super . construct`, or if a derived class has no explicit constructor, `mini-bykig++` automatically chains and invokes zero-argument ancestor constructors up the inheritance hierarchy:

```mini
class BaseAuto
{
    public float status ;
    public func construct ( )
    {
        this . status = 42 ;
    }
}

class ChildAuto extends BaseAuto
{
    public float val ;
}

var ca = new ChildAuto ( ) ;
out ca . status ; // 42 (initialized by BaseAuto constructor)
```

> [!NOTE]
> If any ancestor class in the inheritance chain requires arguments for its constructor and `super . construct` was omitted, construction halts with a diagnostic specifying the required argument count.

### Class Invariants & Validations
`mini-bykig++` validates class declarations to maintain hierarchy invariants:
- **Duplicate Class Rejection**: Defining two classes with the same name outputs `Error: Duplicate class '<Name>'`.
- **Duplicate Field Rejection**: Declaring the same field name twice within the same class or conflicting with an inherited field outputs `Error: Duplicate field '<name>' in class '<Name>'`.
- **Inheritance Cycle Rejection**: Circular or self-referential inheritance (e.g. `class A extends A` or `class A extends B` where `B extends A`) is rejected.

### Null References & Safety
`mini-bykig++` supports explicit null references and safe null state checks:
- **Uninitialized Typed Declarations**: Declaring a typed object variable without assignment initializes it to null:
  ```mini
  Point uninitP ;
  out uninitP ; // null
  ```
- **Explicit `null` Literal**: Variables can be assigned `null`:
  ```mini
  validP = null ;
  ```
- **The `is_null ( expr )` Built-in**: Returns `1` if the value is an uninitialized or null object reference, and `0` for valid object instances or primitive values:
  ```mini
  out is_null ( uninitP ) ; // 1
  out is_null ( validP ) ;  // 1 (after assignment to null)
  out is_null ( 100 ) ;     // 0
  out is_null ( "hello" ) ; // 0
  ```
- **Null Dereference Diagnostics**: Attempting to read/write a member field or invoke a method on a null receiver produces a clean diagnostic rather than undefined behavior:
  ```mini
  // Output: Error: Null reference: cannot access field on null object
  // Output: Error: Null reference: cannot call method on null receiver
  ```

### Method Overriding & Polymorphism
Subclasses can override inherited methods. Method calls resolve dynamically based on the receiver's actual runtime class:

```mini
func makeItSpeak ( animal )
{
    animal . speak ( ) ;
}

var a = new Animal ( "Creature" ) ;
var d = new Dog ( "Buddy" , "Golden Retriever" ) ;

makeItSpeak ( a ) ; // Creature makes a sound.
makeItSpeak ( d ) ; // Buddy barks! (Golden Retriever)
```

### Access Control (`public` & `private`)
- `public`: Accessible anywhere.
- `private`: Strictly accessible only from methods declared within that specific class. Subclasses and external callers cannot read or write private fields, nor invoke private methods:

```mini
class Vault
{
    private secret ;

    public func construct ( code )
    {
        this . secret = code ;
    }

    public func getSecret ( )
    {
        return this . secret ; // Permitted: inside Vault
    }
}

var v = new Vault ( 777 ) ;
out v . getSecret ( ) ; // 777
// out v . secret ;      // Runtime Error: Cannot access private field
```

### Runtime Type Inspection (`is_instance`)
The `is_instance ( obj , "ClassName" )` built-in verifies whether an object is an instance of the given class or any of its parent classes:

```mini
out is_instance ( d , "Dog" ) ;     // 1
out is_instance ( d , "Animal" ) ;  // 1
out is_instance ( d , "String" ) ;  // 0
out is_instance ( 123 , "Dog" ) ;   // 0 (safe for non-objects)
```

### Object String Representation
Objects printed via `out` or converted to strings display their class name and heap ID:

```mini
out d ; // <Object Dog#1>
```

---

## 10. Built-in Functions Reference

| Function | Syntax | Description | Example |
|---|---|---|---|
| `len` | `len(collection)` | Returns the element count of a list or map. | `len([1, 2, 3])` -> `3` |
| `push` | `push(list, val)` | Appends `val` to the end of `list`. | `push(lst, 10)` |
| `pop` | `pop(list)` | Removes and returns the last element of `list`. | `var x = pop(lst)` |
| `exists` | `exists(map, key)` | Returns `1` if `key` exists in `map`, `0` otherwise. | `exists(user, "name")` -> `1` |
| `delete` | `delete(map, key)` | Removes `key` and its associated value from `map`. | `delete(user, "temp")` |
| `is_instance` | `is_instance(obj, "Class")` | Returns `1` if `obj` is an instance of `Class` or a subclass; `0` otherwise. | `is_instance(d, "Animal")` -> `1` |
| `is_null` | `is_null(expr)` | Returns `1` if `expr` is an uninitialized or null object reference; `0` otherwise. | `is_null(uninitP)` -> `1` |

---

## 11. I/O & Execution

### Output (`out`)
The `out` statement prints values or expression results followed by a newline:

```mini
out "Hello, BYKIG++!" ;
out 10 + 20 ;
out [ 1 , 2 , 3 ] ;
```

### Input (`in`)
The `in` statement reads a line of input from standard input into a variable:

```mini
var userInput = "" ;
in userInput ;
out "You entered: " + userInput ;
```

---

## 12. Test Suites

The test suite in [`mini-bykig++/tests/`](tests/) tests all language features:

| File | Subsystems Tested |
|---|---|
| [`test_basic.mini`](tests/test_basic.mini) | Arithmetic operator precedence, string concatenation, variable mutation, booleans. |
| [`test_control.mini`](tests/test_control.mini) | `if`/`elif`/`else`, `while` with `break` & `continue`, `for` loop, block variable shadowing. |
| [`test_func.mini`](tests/test_func.mini) | Multi-argument functions, return values, void procedures, single recursion (`factorial`), nested calls. |
| [`test_collections.mini`](tests/test_collections.mini) | Dynamic lists, generic maps, built-ins (`push`, `pop`, `len`, `exists`, `delete`), 2D arrays, nested indexing. |
| [`test_oop.mini`](tests/test_oop.mini) | Classes, fields, constructors, `this` receiver mutation, inheritance (`extends`), method overriding, polymorphism, `is_instance`, and encapsulation violations. |
| [`test_all.mini`](tests/test_all.mini) | End-to-end regression covering all 9 language subsystems, including double recursion (`fibonacci`). |

### Running the Tests
To execute any test program:

```powershell
echo mini-bykig++/tests/test_oop.mini | .\Interpreter.exe mini-bykig++/main.bykig
```

Or run all tests:

```powershell
echo mini-bykig++/tests/test_all.mini | .\Interpreter.exe mini-bykig++/main.bykig
```
