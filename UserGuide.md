# BYKIG++ User Guide

BYKIG++ is a lightweight interpreter written in C. It executes `.bykig` source directly using token-driven interpretation, linked lists, stacks, environments, and source-position control rather than a traditional AST-based compiler pipeline.

## 1. Syntax rules

BYKIG++ uses clean token spacing.

Correct:

```bykig
int x = 5 ;
```

Incorrect:

```bykig
int x=5;
```

Statements normally end with a spaced semicolon:

```bykig
out "Hello%n" ;
```

A complete program ends with a single dot:

```text
.
```

Use `//` for single-line comments.

For `if`, `elif`, `else`, and `while`, place the opening `{` on the next line:

```bykig
if x > 5
{
    out "Greater%n" ;
}
```

## 2. Primitive types

Supported primitive types:

```text
int
float
char
bool
string
```

Examples:

```bykig
int count = 10 ;
float price = 3.14 ;
char grade = 'A' ;
bool enabled = 1 ;
string name = "BYKIG++" ;
```

## 3. Assignment

The right-hand side can be an expression:

```bykig
int x = 10 ;
int y = x + 5 * 2 ;
```

It can also contain complex expressions such as function calls, built-ins, and collection indexing:

```bykig
int result = add ( 5 , 10 ) ;
float n = len ( myList ) ;
float first = myList [ 0 ] ;
```

## 4. Operators

Arithmetic:

```text
+  -  *  /  %  $
```

Comparison/equality:

```text
<  >  <=  >=  ~  !  ==  !=
```

Logical:

```text
&  |  &&  ||
```

`~` and `==` represent equality; `!` and `!=` represent inequality.

Parentheses are supported:

```bykig
int result = ( 10 + 5 ) * 2 ;
```

## 5. Conditions

```bykig
int x = 10 ;

if x > 5
{
    out "Greater%n" ;
}
elif x ~ 5
{
    out "Equal%n" ;
}
else
{
    out "Smaller%n" ;
}
```

Conditions are evaluated through the expression engine. Truth is represented numerically (`1`/`0`).

## 6. Loops

BYKIG++ supports both `while` loops and `for` loops with full `break` and `continue` control.

### `while` loops

```bykig
int i = 0 ;
while i < 5
{
    out i + "%n" ;
    i = i + 1 ;
}
```

### `for` loops

```bykig
for ( int i = 0 ; i < 5 ; i = i + 1 ) {
    out i + "%n" ;
}
```

`for` loops support:
- Initialization expressions, including variable declarations scoped to the loop (e.g. `int i = 0`) or assignments to existing variables (e.g. `i = 0`).
- Condition expressions (e.g. `i < 5`), or omitted conditions (defaults to true).
- Iteration expressions (e.g. `i = i + 1`), executed at the end of each iteration or upon `continue`.
- Parenthesized or non-parenthesized headers.
- Nested loops and full integration with `break` and `continue`.

Use `break` and `continue` inside loops:

```bykig
while i < 10
{
    if i ~ 5
    {
        break ;
    }
    i = i + 1 ;
}
```

## 7. Functions

Define a function with `func`:

```bykig
func add ( int a , int b )
{
    return a + b ;
}
```

Call it:

```bykig
int result = add ( 5 , 10 ) ;
out result + "%n" ;
```

Functions may also be called as standalone statements where appropriate.

### Recursion

Recursive calls are supported:

```bykig
func fact ( int n )
{
    if n <= 1
    {
        return 1 ;
    }

    return n * fact ( n - 1 ) ;
}

int result = fact ( 5 ) ;
```

The runtime maintains a call stack and creates a fresh environment for each active call.

## 8. Arrays

Primitive arrays are fixed-size collections.

```bykig
intarr numbers 3 ;
numbers [ 0 ] = 10 ;
numbers [ 1 ] = 20 ;
numbers [ 2 ] = 30 ;

float size = len ( numbers ) ;
```

Supported array types:

```text
intarr
floatarr
chararr
boolarr
```

Arrays can participate in complex expression resolution through indexed access.

## 9. Lists

Lists are dynamic collections backed by `Value *data` with automatic capacity doubling. Elements can hold any runtime `Value` (numbers, strings, nested lists, maps):

```bykig
list values = { 10 , 20 , 30 } ;
values [ 0 ] = 100 ;
float x = values [ 1 ] ;
```

Use built-ins for mutation:

```bykig
push ( values , 200 ) ;
float last = pop ( values ) ;
float count = len ( values ) ;
```

Lists are passed to functions by reference, allowing in-place modification.

## 10. Maps

Maps are generic dynamic key-value collections with `Value` storage, supporting string or numeric keys and arbitrary runtime types as values:

```bykig
map user = { "name" : "Alice" , "age" : 30 , "score" : 95.5 } ;
user [ "city" ] = "New York" ;
out user [ "name" ] + "%n" ;
float userAge = user [ "age" ] ;
string userName = user [ "name" ] ;
```

Use built-ins for mutation and queries:

```bykig
push ( user , "country" , "USA" ) ;
float hasCountry = exists ( user , "country" ) ;
string removed = pop ( user , "country" ) ;
delete ( user , "city" ) ;
float total = len ( user ) ;
```

Maps automatically grow their capacity via dynamic reallocation (`realloc`), removing fixed-size limits.

## 11. Nested indexing

BYKIG++ supports multi-dimensional and nested indexing across lists and maps for both reading in expressions and writing in index assignments:

```bykig
// Multi-dimensional lists (nested lists)
list matrix = { { 1 , 2 } , { 3 , 4 } } ;
float val = matrix [ 0 ] [ 1 ] ; // 2
matrix [ 1 ] [ 0 ] = 99 ;

// Deeply nested maps and lists
map database = { "users" : { { "name" : "Bob" , "score" : 50 } } } ;
string name = database [ "users" ] [ 0 ] [ "name" ] ;
database [ "users" ] [ 0 ] [ "score" ] = 100 ;
```

## 12. Built-ins

Collection built-ins:

```text
len ( var )
push ( list , value )
push ( map , key , value )
pop ( list )
pop ( map , key )
exists ( name )
exists ( map , key )
delete ( name )
delete ( map , key )
```

String and value utilities:

```text
quote ( str )               // Encloses string in double quotes
strip_quotes ( str )         // Removes outer double quotes
is_num ( val )              // Returns 1 if numeric, 0 otherwise
is_str ( val )              // Returns 1 if string, 0 otherwise
starts_with ( str , prefix ) // Returns 1 if str begins with prefix
parse_float ( value )       // Converts string to float
```

Filesystem built-ins:

```text
file_exists ( path )
file_write ( path , content )
file_append ( path , content )
file_read ( path )
```

Scanner built-ins:

```text
file_scan_open ( path )
file_scan_next ( )
file_scan_is_str ( )
file_scan_close ( )
file_scan_get_pos ( )
file_scan_set_pos ( position )
```

## 13. Console I/O

Output:

```bykig
string name = "BYKIG++" ;
out "Hello " + name + "!%n" ;
```

Formatting sequences:

```text
%n   newline
%"   double quote
%+   plus symbol
```

Input:

```bykig
int number ;
in number ;
```

## 14. File I/O & Streaming Scanner

Write and append:

```bykig
file_write ( "test.txt" , "Hello" ) ;
file_append ( "test.txt" , " World!" ) ;
```

Read entire file:

```bykig
string data = file_read ( "test.txt" ) ;
out data + "%n" ;
```

Check existence:

```bykig
float ok = file_exists ( "test.txt" ) ;
```

### Streaming scanner

```bykig
float opened = file_scan_open ( "test.txt" ) ;
if opened ~ 1
{
    string token = file_scan_next ( ) ;
    float isString = file_scan_is_str ( ) ;
    out token + "%n" ;
    file_scan_close ( ) ;
}
```

`file_scan_next()` automatically handles:
- Whitespace-delimited words.
- Quoted strings (preserving inner spaces).
- Self-delimiting punctuation tokens: `[`, `]`, `{`, `}`, `(`, `)`, `,`, `;` without requiring whitespace padding.

## 15. Modules

Import another file with `use`:

```bykig
use "mathlib.bykig" ;
```

Paths can be resolved relative to the currently executing source file:

```bykig
use "../common/utils.bykig" ;
```

The imported file is executed by the same interpreter. Its declarations and function definitions become part of the current runtime context rather than a separate module namespace.

## 16. Object-Oriented Programming (OOP)

BYKIG++ includes complete object-oriented programming capabilities integrated directly into its interpreter runtime.

### 16.1 Classes and Fields

Classes define custom object types with fields and methods. Fields default to `public` if no access modifier is specified:

```bykig
class Point
{
    public float x ;
    public float y ;
    string label ;
}
```

Fields can be primitives (`int`, `float`, `char`, `bool`, `string`), arrays, dynamic lists, maps, or references to other objects.

### 16.2 Instantiation and Constructors

Instances are created using the `new` operator. An explicit constructor method is declared using `public func construct ( ... )`:

```bykig
class Point
{
    public float x ;
    public float y ;

    public func construct ( float initialX , float initialY )
    {
        this . x = initialX ;
        this . y = initialY ;
    }
}

Point p = new Point ( 10.5 , 20.0 ) ;
```

If a class does not define a constructor, it provides default zero-argument instantiation (`new ClassName ( )`), zero-initializing all fields. Providing arguments to a class without a matching constructor produces an explicit argument-mismatch error.

### 16.3 Reference Semantics and Aliasing

Object variables hold references to heap-allocated objects rather than copying values:

```bykig
Point p = new Point ( 1.0 , 2.0 ) ;
Point q = p ; // q and p refer to the same instance

q . x = 99.0 ;
out p . x ; // Prints 99.0
```

- **Identity Equality**: `p == q` tests whether `p` and `q` refer to the identical object instance.
- **Null Safety**: Uninitialized object variables (`Point p ;`) or variables assigned `null` (`p = null ;`) are safely checked. Dereferencing a null reference prints an explicit runtime error diagnostic without crashing.
- **Readable Representation**: Printing an object (`out p ;`) outputs its formatted identifier: `<Object Point#1>`.

### 16.4 Instance Methods and `this`

Methods are defined within class bodies and invoked with dot syntax. Within an instance method, `this` refers to the active receiver:

```bykig
class Counter
{
    public float count ;

    public func construct ( float start )
    {
        this . count = start ;
    }

    public func increment ( float amount )
    {
        this . count = this . count + amount ;
        return this . count ;
    }
}

Counter c = new Counter ( 10.0 ) ;
out c . increment ( 5.0 ) ; // 15.0
```

Both spaced (`c . increment ( 5.0 )`) and unspaced (`c.increment ( 5.0 )`) syntax are supported. Receiver state is isolated per method call frame, ensuring nested and recursive method calls do not clobber each other.

### 16.5 Encapsulation (Public and Private Access)

Fields and methods support explicit `public` and `private` modifiers:

- `public`: Accessible from any valid context (inside or outside the class).
- `private`: Accessible only from methods defined within the class declaring the member.

```bykig
class BankAccount
{
    private float balance ;

    public func construct ( float initialBalance )
    {
        this . balance = initialBalance ;
    }

    public func getBalance ( )
    {
        return this . balance ;
    }
}

BankAccount acc = new BankAccount ( 500.0 ) ;
out acc . getBalance ( ) ; // Allowed: 500.0
// acc . balance = 1000.0 ; // Error: Cannot access private member 'balance'
```

### 16.6 Single Inheritance

A class can inherit fields and methods from a superclass using the `extends` keyword. Superclasses must be declared before their derived classes:

```bykig
class Animal
{
    public string name ;

    public func describe ( )
    {
        out "Animal: " + this . name + "%n" ;
    }
}

class Dog extends Animal
{
    public string breed ;

    public func bark ( )
    {
        out "Woof!%n" ;
    }
}

Dog d = new Dog ( ) ;
d . name = "Rex" ;
d . breed = "German Shepherd" ;
d . describe ( ) ; // Inherited method
d . bark ( ) ;     // Subclass method
```

Multilevel inheritance (`class C extends B`, where `B extends A`) is fully supported. Subclasses inherit all accessible superclass fields and methods while preserving object runtime type identity.

### 16.7 Method Overriding and Polymorphism

A subclass can override an inherited method. Overridden methods are dispatched dynamically based on the object's actual runtime type, even when accessed through a parent-typed variable:

```bykig
class Shape
{
    public func area ( )
    {
        return 0.0 ;
    }
}

class Square extends Shape
{
    public float side ;

    public func construct ( float s )
    {
        this . side = s ;
    }

    public func area ( )
    {
        return this . side * this . side ;
    }
}

Shape s = new Square ( 4.0 ) ;
out s . area ( ) ; // Dynamic dispatch invokes Square's area(): 16.0
```

Method override signatures are validated at declaration time: parameter count and parameter types must match the inherited method.

### 16.8 Runtime Type Checks (`is_instance`)

The `is_instance` built-in checks whether an object is an instance of a specified class or any of its subclasses:

```bykig
Animal a = new Dog ( ) ;

out is_instance ( a , "Dog" ) ;    // 1.0 (true)
out is_instance ( a , "Animal" ) ; // 1.0 (true - inherited)
out is_instance ( a , "Square" ) ; // 0.0 (false - unrelated)
```

The class name can be specified as a string literal, string variable, or unquoted identifier. Non-object values, null references, and unknown class names return `0.0` safely without crashing.

## 17. Running BYKIG++

Windows PowerShell:

```powershell
.\Interpreter.exe tests\my_script.bykig
```

Without a file argument, the interpreter prompts for a file name.

### Running mini-bykig++ programs

To execute `.mini` scripts using the self-hosted `mini-bykig++` interpreter:

```powershell
echo mini-bykig++/test_collections.mini | .\Interpreter.exe mini-bykig++/main.bykig
```

## 18. Example program

```bykig
int max ;
in max ;

int i = 2 ;

while i < max + 1
{
    int j = 2 ;
    int composite = 0 ;

    while j < i / 2 + 1
    {
        if i % j ~ 0
        {
            composite = 1 ;
            break ;
        }
        j = j + 1 ;
    }

    if composite ! 1
    {
        out i + "%n" ;
    }

    i = i + 1 ;
}
.
```
