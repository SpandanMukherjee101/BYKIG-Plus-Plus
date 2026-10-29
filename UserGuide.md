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

## 16. Running BYKIG++

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

## 17. Example program

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
