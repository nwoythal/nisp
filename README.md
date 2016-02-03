# Nisp
###Abstract
Nisp (short for Nate's Lisp) is an experiment in Daniel Holden's '[Build Your Own Lisp](https://www.buildyourownlisp.com/)'. The aim of this is to not only have my own personal dialect of lisp, but to further my understanding of languages and exceptions in languages, and to improve upon my C skills. The project is completely finished, though there may be refactoring to add features in the future.

###Installation
These files have dependencies on mpc and editline. MPC can be found at https://github.com/orangeduck/mpc and a simple git clone into the nisp directory should provide everything needed.
To install editline, run either
    su -c "yum install libedit-dev*"
    OR
    sudo apt-get install libedit-dev

To compile on Linux, just run the script provided, it will link all the necessary files, as well as update the output file. There is currently no support for Windows.

###About
Nisp is a basic implementation of lisp. It supports basic double types, variables, and functions. It can also perform basic operatations including add, subtract, mutliply, divide, modulus, and exponentials. There is also support for compound list types and functions for those, including list combination, tail, head, and value evaluation. The things I have learned from this assignment include grammar definitions, stack layout (through gdb), functional programming (through implementing currying), as well as exception handling.
I have tested the program through provided example functions, including
    
    load "stdlib.nsp"
    def {add-two}  (\ {x y} {+ x y})
    curry + {1 2 3 4 5}
    uncurry head 5 6 7
    (+ 3 (- 4 1))
    pow 8 2

I have developed components for function and variable creation, as well as parsing user input (albeit from an external library). To test it, feel free to utilize the functions above to test and run the program.

The main modules of the project include the Repl in the main method, which is constantly executed until the user inserts an interrupt. The builtins functions, while not entirely modular, are frequently used in execution.

In terms of linking among abstractions, most of these things are a type of Lisp Value (lval) structure, which can be extended to support errors and compound expressions. This leads to minimal useage of memory, as well as dynamic typecasting based on user input. The program is able to determine whether or not the data is an atomic type or compound value, and speculate based on that.

##Basic usage of the language:
####Datatypes:
Integer
```
1
8
888888888888
```
Float (up to 3 decimal places)  
`3.14`    (Returns `3.14`)  
`18.0`    (Returns `18`)  
`18.0001` (Returns `18.000`)  
`3.14195` (Returns `3.141`)  
String  
`"Hello, world!"`  
List  
```
{1 2 3 4}
{0.1 0.12 0.123}
list "Hello" ", " "world" "!"
```
####Operations
```
op value value ...  
+ 1 2
list "Hello" ", " "world" "!"
head {1 2 3 4}
```
See below for a list of all builtin operations  
def {var} {value}  
```
def {x} 1
def {name} "Nate"
```
Also works for functions
```
def {sqrt} (\ {x} {pow x .5})
def {percent-error} (\ {x y} {* (/ (- x y) x) 100})
```
###Default Builtin Functions
Mathematical: `+`, `-`, `/`, `*`, `^`, `%` (Can also be called via `add`, `sub`, `div`, `mul`, `pow`, `mod`)  
List Operations: `head`, `tail`, `list`, `eval`, `join`  
Declarations: `def`  
Logical: `if`, `>`, `>=`, `<`, `<=`, `==`, `!=`, `greater`, `less`, `equal`  
###Included in stdlib.nsp
Atomic types: `nil`, `true`, `false`  
Declarations: `fun`  
Packing & unpacking: `unpack`, `pack`, `curry`, `uncurry`  
Sequential operations: `do`  
Scope definition: `let`  
Logical operations: `not`, `or`, `and`  
List operations: `first`, `second`, `third`, `nth`, `last`, `len` 
