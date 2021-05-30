Project4 - Phase3

functions of shell :
    shell can process background jobs for both piped and unpiped(single command).
    shell can control jobs by jobs, bg, fg and kill, which are builtin commands.

myshell.{h,c} :
    main source and header file to execute myshell for Phase3.
    PG : structure for custom Process Group.
    BGPGs, FGPGs : doubly linked list for PG.
    several functions to support PG list.

myshell_func.c  :
    source file that includes implementation of functions defined in myshell.h.
    functions of logics for the Phase.
    exec_pipe() : generate pipes according to the number of commands.
                  call pipe_fork_exec() and free memory.
    pipe_fork_execve() : Fork() processes according to the number of commands.
                       link pipes between processes with closing and duplicating proper end of pipe - either READ or WRITE or both-.
    search_and_execve() : Try to execute /bin/FILE first if failed try /usr/bin/FILE
    SIGINT, SIGTSTP, SIGCHLD handlers : custom signal handlers.

csapp.{h,c} :
    header file that defines functions.
    source file that includes implementation of functions defined in csapp.h.
    implemented by Fork() and Execve() which are wrapping system call - fork and execve -.
    excluding exit, &, cd which are built-in commands not calling Fork() and Execve().

