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
 * - parseline : add to parse pipe mark
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
    int cmd_index[MAXARGS] = {-1,};

    strcpy(buf, cmdline);
    bg = parseline(buf, argv, cmd_index, &pipe_count);
    if (argv[0] == NULL)
        return;   /* Ignore empty lines */
    if (!builtin_command(argv)) { // quit -> exit(0), & -> ignore, other -> run
        if (pipe_count > 0) {
            exec_pipe(argv, cmd_index, pipe_count);
        } else {
            /* concat /bin/ in front of argv[0] */
            if (strncmp("/bin/", argv[0], 5) != 0) {
                char temp[MAXBUF];
                strcpy(temp, "/bin/");
                strcat(temp, argv[0]);
                strcpy(argv[0], temp);
            }
            if((pid = Fork()) == 0) { /* Child runs user job */
                Execve(argv[0], argv, environ); // ex) /bin/ls ls -al &
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
int parseline(char *buf, char **argv, int index[], int* pipe_count)
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */
    int idx = 0;

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    index[idx++] = argc;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        /* Count parse */
        if (strcmp(argv[argc-1], "|") == 0) {
            index[idx++] = argc;
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
int exec_pipe(char** argv, int index[], const int pipe_count) {
    int fd[2];
    pid_t *pid;
    char ***temp_argv;
    int n = 0;

    pid = (pid_t *) Malloc(sizeof(pid_t) * (pipe_count + 1));

    temp_argv = (char ***) Malloc(sizeof(char **) * (pipe_count+1));

    for (int j = 0; j <= pipe_count; ++j) {
        /* Allocate Dynamic CHAR array for argv of each command */
        temp_argv[j] = (char**) Malloc(sizeof(char*)*MAXARGS);
        for (int i = 0; i < MAXARGS; ++i) {
            temp_argv[j][i] = (char *) Malloc(sizeof(char) * 64);
        }

        /* copy argv to temp argv */
        int i = 0;
        while (argv[n] != NULL) {
            if(strcmp(argv[n], "|") == 0) {
                n++;
                break;
            }
            strcpy(temp_argv[j][i++], argv[n++]);
        }
        /* concat /bin/ in front of each command */
        if (strncmp("/bin/", temp_argv[j][0], 5) != 0) {
            char bin[MAXBUF] = {0,};
            strcpy(bin, "/bin/");
            strcat(bin, temp_argv[j][0]);
            strcpy(temp_argv[j][0], bin);
        }
        temp_argv[j][i] = NULL;
    }

    /* Make pipe */
    if (pipe(fd) < 0) {
        fprintf(stdout, "pipe error : %s\n", strerror(errno));
        return -1;
    }
    n = 0;
    if ((pid[n++] = Fork()) == 0) { // First Command
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        Execve(temp_argv[n - 1][0], temp_argv[n - 1], environ);
    }
    else {
        if ((pid[n] = Fork()) == 0) { // Second Command
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            Execve(temp_argv[n][0], temp_argv[n], environ);
        }
        else { // Parent reaps second command process
            close(fd[1]);
            int status;
            Waitpid(pid[n - 1], &status, 0);
        }
        // Parent reaps first command process
        int status;
        Waitpid(pid[n - 2], &status, 0);
    }

    for (int i = 0; i <= pipe_count; ++i) {
        for (int j = 0; j < MAXARGS; ++j) {
            free(temp_argv[i][j]);
        }
        free(temp_argv[i]);
    }
    free(temp_argv);
    return 1;
}
/* $end exec_pipe */
