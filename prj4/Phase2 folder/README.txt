Project4 - Phase1
myshell.{h,c} :
    main source and header file to execute myshell for Phase1.
    implemented by Fork() and Execve() which are wrapping system call - fork and execve -.
    excluding exit, &, cd which are built-in commands not calling Fork() and Execve().

myshell_func.c  :
    source file that includes implementation of functions defined in myshell.h.

csapp.{h,c} :
    header file that defines functions.
    source file that includes implementation of functions defined in csapp.h.

