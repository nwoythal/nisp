This file is an experiment in Daniel Holden's 'Build Your Own Lisp'. The aim of this is to not only have my own personal dialect of lisp, but to further my understanding of languages and exceptions in languages, and to improve upon my C skills. I have completed up through chapter 11.

These files have dependencies on mpc and editline. MPC can be found at https://github.com/orangeduck/mpc and a simple git clone into the nisp directory should provide everything needed.
To install editline, run either
    su -c "yum install libedit-dev*"
    OR
    sudo apt-get install libedit-dev

To compile on Linux, just run the script provided, it will link all the necessary files, as well as update the output file. There is currently no support for Windows.

About this file:
It is a basic implementation of lisp. It supports basic double types, variables, and functions. It can also perform basic operatations including add, subtract, mutliply, divide, modulus, and exponentials. There is also support for compound list types and functions for those, including list combination, tail, head, and value evaluation. The things I have learned from this assignment include grammar definitions, stack layout (through gdb), functional programming (through implementing currying), as well as exception handling.
I have tested the program through provided example functions, including

    def {add-two}  (\ {x y} {+ x y})
    def {fun} (\ {args body} {def (head args) (\ (tail args) body)})
    fun {unpack f xs} {eval (join (list f) xs)}
    fun {pack f & xs} {f xs}
    def {uncurry} pack
    def {curry} unpack
    curry + {1 2 3 4 5}
    uncurry head 5 6 7
    (+ 3 (- 4 1))
    pow 8 2

I have developed components for function and variable creation, as well as parsing user input (albeit from an external library). The project is slightly incomplete, as there is no support for other atomic values including characters or booleans. To test it, feel free to utilize the functions above to test and run the program.

The main modules of the project include the Repl in the main method, which is constantly executed until the user inserts an interrupt. The builtins functions, while not entirely modular, are frequently used in execution.

In terms of linking among abstractions, most of these things are a type of Lisp Value (lval) structure, which can be extended to support errors and compound expressions. This leads to minimal useage of memory, as well as dynamic typecasting based on user input. The program is able to determine whether or not the data is an atomic type or compound value, and speculate based on that.
