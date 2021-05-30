Project4 - Phase2

functions of shell :
    shell can process piped commands.

myshell.{h,c} :
    main source and header file to execute myshell for Phase2.

myshell_func.c  :
    source file that includes implementation of functions defined in myshell.h.
    functions of logics for the Phase.
    exec_pipe() : generate pipes according to the number of commands.
                  call pipe_fork_exec() and free memory.
    pipe_fork_execve() : Fork() processes according to the number of commands.
                       link pipes between processes with closing and duplicating proper end of pipe - either READ or WRITE or both-.
    search_and_execve() : Try to execute /bin/FILE first if failed try /usr/bin/FILE

csapp.{h,c} :
    header file that defines functions.
    source file that includes implementation of functions defined in csapp.h.
    implemented by Fork() and Execve() which are wrapping system call - fork and execve -.
    excluding exit, &, cd which are built-in commands not calling Fork() and Execve().

