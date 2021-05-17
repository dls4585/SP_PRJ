/*
 * myshell_func.c - main function for myshell
 *
 * Updated 2021/05/16 - 백인찬
 * - Changed functions to wrapping functions. ex) execve() -> Execve()
 * - Add reference of "/bin/" to argv[0]
 *   ,which makes user not to type /bin/ explicitly before shell command.
 *   ex) /bin/ls -tal -> ls -tal
 * - Add cd command to builtin_command() and implement cd function.
 *
 * Updated 2021/05/16 - 백인찬
 * - parseline : add to parse pipe mark.
 *
 * Updated 2021/05/17 - 백인찬
 * - exec_pipe(), search_and_execve(), pipe_fork_execve() : newly added functions for pipe.
 */

#include "csapp.h"
#include "myshell.h"

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline)
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    int pipe_count = 0; /* Number of Pipe in command line */
    char bin[MAXBUF] = {0,};
    char usr[MAXBUF] = {0,};

    strcpy(buf, cmdline);
    bg = parseline(buf, argv, &pipe_count);
    if (argv[0] == NULL)
        return;   /* Ignore empty lines */
    if (!builtin_command(argv)) { // quit -> exit(0), & -> ignore, other -> run
        if (pipe_count > 0) {
            exec_pipe(argv, pipe_count);
        } else {
            /* concat /bin/ in front of argv[0] */
            if (strncmp("/bin/", argv[0], 5) != 0) {
                strcpy(bin, "/bin/");
                strcat(bin, argv[0]);
                argv[0] = bin;
            }
            if((pid = Fork()) == 0) { /* Child runs user job */
                /* if executable file does not exist in /bin/,
                    try /usr/bin */
                if(execve(argv[0], argv, environ) < 0) { // ex) /bin/ls ls -al &
                    strcpy(usr, "/usr");
                    strcat(usr, argv[0]);
                    argv[0] = usr;
                    Execve(argv[0], argv, environ);
                }
            }

            /* Parent waits for foreground job to terminate */
            if (!bg){
                int status;
                Waitpid(pid, &status, 0);
            }
            else //when there is backgrount process!
                printf("%d %s", pid, cmdline);
        }
    }
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "exit")) /* exit command */
        exit(0);
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
        return 1;
    if (!strcmp(argv[0], "cd")) {
        cd(argv[1]);
        return 1;
    }

    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv, int* pipe_count)
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        if(argv[argc-1][0] == '\"' || argv[argc-1][0] == '\'') {
            argv[argc-1]++;
            *(delim - 1) = '\0';
        }
        else *delim = '\0';
        /* Count parse */
        if (strcmp(argv[argc-1], "|") == 0) {
            (*pipe_count)++;
        }
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0)  /* Ignore blank line */
        return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}
/* $end parseline */

/* $begin cd */
void cd(char* path) {
    if (chdir(path) < 0) /* Changed directory to PATH */
        fprintf(stdout, "cd failed: %s\n", strerror(errno));
}
/* $end cd */

/* $begin exec_pipe */
int exec_pipe(char** argv, const int pipe_count) {
    pid_t *pid;
    char ***pipe_argv;
    int n = 0;

    pid = (pid_t *) Malloc(sizeof(pid_t) * (pipe_count + 1));

    pipe_argv = (char ***) Malloc(sizeof(char **) * (pipe_count + 1));

    for (int j = 0; j <= pipe_count; ++j) {
        /* Allocate Dynamic CHAR array for argv of each command */
        pipe_argv[j] = (char**) Malloc(sizeof(char*) * MAXARGS);
        for (int i = 0; i < MAXARGS; ++i) {
            pipe_argv[j][i] = (char *) Malloc(sizeof(char) * 64);
        }

        /* copy argv to temp argv */
        int i = 0;
        while (argv[n] != NULL) {
            if(strcmp(argv[n], "|") == 0) {
                n++;
                break;
            }
            strcpy(pipe_argv[j][i++], argv[n++]);
        }
        /* concat /bin/ in front of each command */
        if (strncmp("/bin/", pipe_argv[j][0], 5) != 0) {
            char bin[MAXBUF] = {0,};
            strcpy(bin, "/bin/");
            strcat(bin, pipe_argv[j][0]);
            strcpy(pipe_argv[j][0], bin);
        }
        pipe_argv[j][i] = NULL;
    }

    /* Make pipe */
    int **fds = (int **) Malloc(sizeof(int *) * pipe_count);
    for (int i = 0; i < pipe_count; ++i) {
        fds[i] = (int *) Malloc(sizeof(int) * 2);
        if(pipe(fds[i]) < 0) {
            fprintf(stdout, "pipe error : %s\n", strerror(errno));
            return -1;
        }
    }

    pipe_fork_execve(pipe_argv, pid, fds, pipe_count);

    /* free dynamically allocated variables */
    for (int i = 0; i < pipe_count; ++i) {
        free(fds[i]);
    }
    free(fds);

    for (int i = 0; i <= pipe_count; ++i) {
        for (int j = 0; j < MAXARGS; ++j) {
            free(pipe_argv[i][j]);
        }
        free(pipe_argv[i]);
    }
    free(pipe_argv);
    return 1;
}
/* $end exec_pipe */


/* $begin pipe_fork_execve */
void pipe_fork_execve(char ***argv, int *pid, int **fds, int pipe_count) {
    int status;
    for (int i = 0; i <= pipe_count; ++i) {
        pid[i] = Fork();
        /* First command */
        if(i == 0) {
            if (pid[i] == 0) { // First child process runs first command
                close(fds[i][READ]); // close READ end of pipe
                dup2(fds[i][WRITE], STDOUT_FILENO); // duplicate STDOUT as WRITE end of pipe
                close(fds[i][WRITE]); // close WRITE end of pipe
                search_and_execve(argv[i][0], argv[i]);
            }
            else { // Process reaps first child
                Waitpid(pid[i], &status, 0);
            }
        }
            /* Last command */
        else if (i == pipe_count) {
            if(pid[i] == 0) { // Last child process runs last command
                close(fds[i - 1][WRITE]); // close WRITE end of pipe
                dup2(fds[i - 1][READ], STDIN_FILENO); // duplicate STDIN as READ end of pipe
                close(fds[i - 1][READ]); // close READ end of pipe
                search_and_execve(argv[i][0], argv[i]);
            }
            else { // Process reaps last child
                close(fds[i - 1][WRITE]);
                Waitpid(pid[i], &status, 0);
            }
        }
            /* commands in the middle */
        else {
            if(pid[i] == 0) { // middle child process runs command
                close(fds[i - 1][WRITE]); // close WRITE end of pipe connected with BEFORE command
                dup2(fds[i - 1][READ], STDIN_FILENO); // duplicate STDIN as READ end of pipe
                close(fds[i - 1][READ]); // close READ end of pipe

                close(fds[i][READ]); // close READ end of pipe connected with NEXT command
                dup2(fds[i][WRITE], STDOUT_FILENO); // duplicate STDOUT as WRITE end of pipe
                close(fds[i][WRITE]); // close WRITE end of pipe

                search_and_execve(argv[i][0], argv[i]);
            }
            else { // Process reaps last child
                close(fds[i - 1][WRITE]);
                Waitpid(pid[i], &status, 0);
            }
        }
    }
}
/* $end pipe_fork_execve */

/* $begin search_and_execve */
void search_and_execve(char* filename, char** argv) {
    /* if executable file does not exist in /bin/,
       try /usr/bin  */
    if(execve(filename, argv, environ) < 0) {
        char usr[MAXBUF] = {0,};
        strcpy(usr, "/usr");
        strcat(usr, filename);
        strcpy(filename, usr);
        Execve(filename, argv, environ);
    }
}
/* $end search_and_execve */


