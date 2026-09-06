# BYKIG++ User Guide

BYKIG++ is a lightweight interpreter written in C. It executes `.bykig` source directly using token-driven interpretation, linked lists, stacks, environments, and source-position control rather than a traditional AST-based compiler pipeline.

## 1. Syntax rules

BYKIG++ uses strict token spacing.

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

It can also contain complex expressions such as function calls, built-ins, and collection indexing where supported:

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
<  >  <=  >=  ~  !
```

Logical:

```text
&  |
```

`~` means equality and `!` means not-equal in BYKIG++ expression syntax; do not use C-style `==` or `!=` as the language operators.

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

Only `while` loops are implemented.

```bykig
int i = 0 ;
while i < 5
{
    out i + "%n" ;
    i = i + 1 ;
}
```

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

There is no `for` loop in the current language.

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

Lists are dynamic numeric collections:

```bykig
list values = { 10 , 20 , 30 } ;
values [ 0 ] = 100 ;
float x = values [ 1 ] ;
```

Use built-ins for mutation:

```bykig
push ( "values" , 200 ) ;
float last = pop ( "values" ) ;
```

The current implementation stores list elements as `float` values.

## 10. Maps

Maps use string keys and numeric values:

```bykig
map users = { "alice" = 40 , "bob" = 50 } ;
users [ "carol" ] = 60 ;
float age = users [ "alice" ] ;
```

The map implementation uses a fixed table of up to 100 entries with linear key lookup.

## 11. Built-ins

Common collection built-ins:

```text
len ( var )
push ( collection , value )
pop ( collection )
exists ( ... )
delete ( ... )
parse_float ( value )
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
file_scan_close ( )
file_scan_get_pos ( )
file_scan_set_pos ( position )
```

## 12. Console I/O

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

## 13. File I/O

Write and append:

```bykig
file_write ( "test.txt" , "Hello" ) ;
file_append ( "test.txt" , " World!" ) ;
```

Read:

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
    out token + "%n" ;
    file_scan_close ( ) ;
}
```

`file_scan_next()` exposes the next whitespace-delimited token through the built-in string-return path and returns a success/failure numeric result.

## 14. Modules

Import another file with `use`:

```bykig
use "mathlib.bykig" ;
```

Paths can be resolved relative to the currently executing source file:

```bykig
use "../common/utils.bykig" ;
```

The imported file is executed by the same interpreter. Its declarations and function definitions become part of the current runtime context rather than a separate module namespace.

## 15. Running BYKIG++

Windows PowerShell:

```powershell
.\Interpreter.exe tests\my_script.bykig
```

Without a file argument, the interpreter prompts for a file name.

## 16. Example program

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
