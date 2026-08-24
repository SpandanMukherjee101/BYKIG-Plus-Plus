# BYKIG++ Documentation

Made by Spandan Mukherjee
*Completion Date: 22/08/26*
*Dedicated to: My mother TANDRA MUKHERJEE*

BYKIG++ is a lightweight, dynamically parsed interpreter built in C entirely using raw Data Structures (Linked Lists and Stacks), bypassing traditional Compiler Design concepts (no ASTs, Lexers, or Parsers). 

---

## 🚀 Feature Support Matrix

The following tables detail exactly what features are supported and in what contexts they can be used.

### 1. Operations & Built-ins vs. Data Types

| Operation / Feature (What's Supported) | `int` / `float` / `char` / `bool` | `string` | Primitive Arrays | `list` / `map` | Functions |
| --- | :---: | :---: | :---: | :---: | :---: |
| **Declaration** (`type x ;`) | ✅ | ✅ | ✅ | ✅ | ✅ (`func`) |
| **Assignment** (`x = ... ;`) | ✅ | ✅ | ❌ (Index only) | ❌ | ❌ |
| **Console Output** (`out x ;`) | ✅ (Except `bool`) | ✅ | ❌ | ❌ | ❌ |
| **Console Input** (`in x ;`) | ✅ (Except `bool`) | ❌ | ❌ | ❌ | ❌ |
| **Arithmetic** (`+`, `-`, `*`, `/`, `%`) | ✅ | ❌ | ❌ | ❌ | ❌ |
| **Logical/Relational** (`~`, `!`, `<`, `>`) | ✅ | ❌ | ❌ | ❌ | ❌ |
| **String Concatenation** (`+`) | ❌ | ✅ (In `out` only) | ❌ | ❌ | ❌ |
| **Bracket Indexing** (`arr[0]`) | ❌ | ❌ | ✅ | ❌ | ❌ |
| **`len()` Built-in** | ❌ | ✅ | ✅ | ✅ | ❌ |
| **`push()` / `pop()`** | ❌ | ❌ | ❌ | ✅ | ❌ |
| **`exists()` / `delete()`** | ✅ | ✅ | ✅ | ✅ | ❌ |
| **Returnable** (`return x ;`) | ✅ | ✅ | ❌ | ❌ | ❌ |
| **Pass as Argument** | ✅ | ✅ | ❌ | ❌ | ❌ |

### 2. Statements & Features vs. Scopes

| Feature / Statement (What's Supported) | Global Scope | Function Body | `if` / `while` Block | Imported Module (`use`) | Expression RHS (`=`) | `out` Statement |
| --- | :---: | :---: | :---: | :---: | :---: | :---: |
| **Variable Declaration** | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |
| **Variable Assignment** | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |
| **Math Expressions** (`+`, `-`, `*`) | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ |
| **Logic Expressions** (`~`, `<`, `>`) | ❌ | ❌ | ✅ (In condition) | ❌ | ✅ | ❌ |
| **Standalone Function Call** | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |
| **Assigned Function Call** | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ |
| **Standalone Built-ins** (`push`) | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |
| **Assigned Built-ins** (`pop`, `len`) | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ |
| **Control Flow** (`if`, `while`) | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ |
| **`return` Statement** | ❌ | ✅ | ✅ | ❌ | ❌ | ❌ |
| **String Concatenation** (`+`) | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |

---

## 1. Syntax Basics

BYKIG has very strict syntax rules regarding spacing and program termination.

*   **Spacing:** ALMOST EVERY token must be separated by a single space. This includes variables, operators, brackets, and semicolons.
    *   *Correct:* `int x = 5 ;`
    *   *Incorrect:* `int x=5;`
*   **Termination:** Every statement must end with a spaced semicolon ` ;`.
*   **End of File:** The entire program MUST end with a single dot `.`.
*   **Comments:** Use `//` for single-line comments.
    *   `// This is a comment`

## 2. Datatypes & Variables

BYKIG supports 5 core primitive types:
*   `int` - Integers
*   `float` - Decimal numbers
*   `char` - Single characters (use single quotes `'a'`)
*   `bool` - Booleans (`1` or `0`)
*   `string` - Text (use double quotes `"hello"`)

### Declaration and Assignment
```bykig
int x = 10 ;
float y = 3.14 ;
char c = 'A' ;
bool isTrue = 1 ;
string msg = "Hello World" ;
```

## 3. Operators & Arithmetic

Mathematical expressions support standard BODMAS priority. Parentheses are supported but must be spaced.
*   **Arithmetic:** `+`, `-`, `*`, `/`, `%`
*   **Equality:** `~` (Equals), `!` (Not Equals)
    *   *Note: Do not use `==` or `!=`.*
*   **Comparison:** `<`, `>`, `<=`, `>=`

```bykig
int result = ( 10 + 5 ) * 2 ;
```

## 4. Conditions

Use `if` and `else`. Note the required curly braces on new lines.

```bykig
int a = 10 ;
if a > 5
{
    out "Greater%n" ;
}
else
{
    out "Smaller%n" ;
}
```

## 5. Loops

BYKIG supports `while` loops. 
*Note: `for` loops are not implemented.*

```bykig
int i = 0 ;
while i < 5
{
    out i + "%n" ;
    i = i + 1 ;
}
```

## 6. Functions & Recursion

Declare functions using the `func` keyword. Functions can take arguments and return values using `return`. Functions can be executed as part of an assignment or as standalone global calls.

```bykig
func add ( int a , int b )
{
    int c = a + b ;
    return c ;
}

int res = add ( 5 , 10 ) ;
out res + "%n" ;

// Standalone execution
main ( ) ;
```

### Recursion & Stack Overflow
Recursion is fully supported. However, to prevent hanging or memory leaks, BYKIG enforces a `MAX_CALL_STACK` limit of 999 frames. Exceeding this triggers a runtime `Error: Stack Overflow`.

## 7. Scopes

BYKIG uses a global environment stack (`envStack`). 
Variables declared inside a function or a block are pushed to the current scope level and are destroyed when the block exits. Variable lookups cascade downwards, meaning local variables shadow global ones.

## 8. Arrays (Primitives)

Declare arrays of primitives. Arrays have fixed size on declaration.
*Types:* `intarr`, `floatarr`, `chararr`, `boolarr`.

```bykig
intarr numbers 5 ;
numbers [ 0 ] = 10 ;
numbers [ 1 ] = 20 ;

int length = len ( numbers ) ;
```

## 9. Strings & Console I/O

### Output (`out`)
String printing supports variable concatenation.
*   `%n` - Newline
*   `%"` - Double Quote
*   `%+` - Plus symbol

```bykig
string name = "BYKIG" ;
out "Welcome to " + name + "!!!%n" ;
```

### Input (`in`)
Accepts user input from the console into variables.

```bykig
int num ;
in num ;
```

## 10. Collections (Lists & Maps)

BYKIG implements dynamic linked-lists (`list`) and key-value maps (`map`). These are manipulated via built-in API functions rather than bracket notation.

### Lists
```bykig
list myList ;
push ( "myList" , 100 ) ;
push ( "myList" , 200 ) ;
pop ( "myList" ) ;
```

### Maps
```bykig
map myMap ;
push ( "myMap" , "key1" , 500 ) ;
float val = pop ( "myMap" , "key1" ) ;
```

## 11. Built-in Functions

The following global built-ins operate on primitives and collections:
*   `len ( var )` - Returns the length of strings, arrays, lists, or maps.
*   `exists ( var_name_string )` - Returns `1` if a variable exists in scope, `0` otherwise.
*   `delete ( var_name_string )` - Destroys the variable from memory.
*   `push` - Appends data to collections.
*   `pop` - Removes data from collections.

## 12. File I/O

Read and manipulate standard files. `file_read` returns a string. All commands require string paths.

```bykig
file_write ( "test.txt" , "Hello" ) ;
file_append ( "test.txt" , " World!" ) ;

string data = file_read ( "test.txt" ) ;
out data + "%n" ;

float check = file_exists ( "test.txt" ) ;
```

## 13. Modules

Import other `.bykig` files using the `use` keyword. This evaluates the file and imports its global variables and functions into your current environment.

```bykig
use "mathlib.bykig" ;
```

## 14. CLI Usage

To run a BYKIG script, execute the interpreter and pass the file path as an argument.

**Windows PowerShell:**
```bash
.\Interpreter.exe tests\my_script.bykig
```
**Interactive Mode:**
If you run `.\Interpreter.exe` without arguments, it will prompt you for the file name.

## 15. Examples

### Prime Number Checker
```bykig
int max , b = 0 , j , i = 2 ;
out "Enter max limit:%n" ;
in max ;

while i < max + 1
{
    int j = 2 ;
    while j < i / 2 + 1
    {
        if i % j ~ 0
        {
            int b = 1 ;
        }
        j = j + 1 ;
    }
    
    if b ! 1
    {
        out i + "%n" ;
    }

    int b = 0 ;
    i = i + 1 ;
}
.
```
