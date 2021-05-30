Project4 - Phase1

functions of shell :
    shell can process unpiped(single) command.

myshell.{h,c} :
    main source and header file to execute myshell for Phase1.

myshell_func.c  :
    source file that includes implementation of functions defined in myshell.h.
    functions of logics for the Phase.

csapp.{h,c} :
    header file that defines functions.
    source file that includes implementation of functions defined in csapp.h.
    implemented by Fork() and Execve() which are wrapping system call - fork and execve -.
    excluding exit, &, cd which are built-in commands not calling Fork() and Execve().

