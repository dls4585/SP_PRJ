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
 *
 * Updated 2021/05/19 - 백인찬
 * - implemented background process for un-piped command with &.
 *
 * Updated 2021/05/20 - 백인찬
 * - implemented background process for piped command with &.
 * - implemented builtin command : BGjobs
 *
 * Updated 2021/05/23 - 백인찬
 * - implemented builtin command : bg
 * - explicitly reaped when !bg.
 *
 * Updated 2021/05/27 - 백인찬
 * - changed data structure for process group.
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

    int* pids;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv, &pipe_count);
    if (argv[0] == NULL)
        return;   /* Ignore empty lines */
    if (!builtin_command(argv)) { // quit -> exit(0), & -> ignore, other -> run
        Sigprocmask(SIG_BLOCK, &mask_one, &prev_one);
        if(BGPGs->count != 0) {
            Sigprocmask(SIG_SETMASK, &prev_one, NULL);
        }
        if (pipe_count > 0) {

            if((pids = exec_pipe(argv, pipe_count, bg, cmdline)) != NULL) {
                if (!bg) {
                    /* Parent waits for foreground job to terminate */

                } else {
                    //when there is backgrount process!
                    printf("%d\n", pids[pipe_count]);
                    return;
                }
            }
        } else {
            if ((pid = Fork()) == 0) { /* Child runs user job */
                Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                Signal(SIGINT, SIG_DFL);
                Signal(SIGTSTP, SIG_DFL);
                /* concat /bin/ in front of argv[0] */
                if (strncmp("/bin/", argv[0], 5) != 0) {
                    strcpy(bin, "/bin/");
                    strcat(bin, argv[0]);
                    argv[0] = bin;
                }
                /* if executable file does not exist in /bin/,
                    try /usr/bin */
                if (execve(argv[0], argv, environ) < 0) { // ex) /bin/ls ls -al &
                    strcpy(usr, "/usr");
                    strcat(usr, argv[0]);
                    argv[0] = usr;
                    Execve(argv[0], argv, environ);
                }
            }
            else {
                if (!bg) {
                    /* Parent waits for foreground job to terminate */
                    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                    int id = FGPGs->count == 0 ? 1 : FGPGs->tail->job_id + 1;
                    PG *pg = PG_create(id, 1, cmdline);
                    pg->pids[0] = pid;
                    PG_push_back(FGPGs, pg);
                    reaped = 0;
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                    while(!reaped);
                } else {
                    //when there is backgrount process!
                    printf("%d %s", pid, cmdline);
                    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                    int id = BGPGs->count == 0 ? 1 : BGPGs->tail->job_id + 1;
                    PG *pg = PG_create(id, 1, cmdline);
                    pg->pids[0] = pid;
                    PG_push_back(BGPGs, pg);
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                    return;
                }
            }
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
    if(!strcmp(argv[0], "jobs")) {
        PG_print(BGPGs);
        return 1;
    }
    if(!strcmp(argv[0], "bg")) {
        int id = (int) strtol(argv[1] + 1, NULL, 10);
        PG *pg = PG_search(BGPGs, id, 0, S_JOBID);
        if(pg->status == STOPPED) {
            change_PG_status(BGPGs, pg, RUNNING);
            for (int i = 0; i < pg->count; ++i) {
                kill(pg->pids[i], SIGCONT);
            }
        }
        else if (pg->status == RUNNING){
            printf("bg: job %d already in background\n", id);
        }
        else {
            printf("bg: %s: no such job\n", argv[1]);
        }
        return 1;
    }
    if(!strcmp(argv[0], "fg")) {
        int status;
//        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
        int id = (int) strtol(argv[1] + 1, NULL, 10);
        PG* pg = PG_search(BGPGs, id, 0, S_JOBID);
        if(pg->status == STOPPED) {
            change_PG_status(BGPGs, pg, RUNNING);
        }
        PG_delete(BGPGs, pg);
        pg->job_id = FGPGs->count == 0 ? 1 : FGPGs->tail->job_id + 1;
        PG_push_back(FGPGs, pg);
        reaped = 0;
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
        for (int i = 0; i < pg->count; ++i) {
            kill(pg->pids[i], SIGCONT);
        }
        while(!reaped);
        return 1;
    }
    if(!strcmp(argv[0], "fgjobs")) {
        PG_print(FGPGs);
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
    char* bgptr;
    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Should the job run in the background? */
    if ((bgptr = strchr(buf,'&')) != NULL) {
//        argv[--argc] = NULL;
        *bgptr = ' ';
        bg = 1;
    } else
        bg = 0;

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
int* exec_pipe(char** argv, const int pipe_count, const int bg, char* cmdline) {
    pid_t *pid;
    char ***pipe_argv;
    int n = 0;
    PG* pg;

    if(!bg) {
        int id = FGPGs->count == 0 ? 1 : FGPGs->tail->job_id + 1;
        pg = PG_create(id, pipe_count + 1, cmdline);
        PG_push_back(FGPGs, pg);
    }
    else {
        int id = BGPGs->count == 0 ? 1 : BGPGs->tail->job_id + 1;
        pg = PG_create(id, pipe_count + 1, cmdline);
        PG_push_back(BGPGs, pg);
    }

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
            return NULL;
        }
    }

    pipe_fork_execve(pipe_argv, pid, fds, pipe_count, bg, pg);

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
    return pid;
}
/* $end exec_pipe */


/* $begin pipe_fork_execve */
void pipe_fork_execve(char ***argv, int *pid, int **fds, int pipe_count, const int bg, PG* pg) {
    int status;
    for (int i = 0; i <= pipe_count; ++i) {
        pid[i] = Fork();
        /* First command */
        if(i == 0) {
            if (pid[i] == 0) { // First child process runs first command
                Signal(SIGINT, SIG_DFL);
                Signal(SIGTSTP, SIG_DFL);
                Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                close(fds[i][READ]); // close READ end of pipe
                dup2(fds[i][WRITE], STDOUT_FILENO); // duplicate STDOUT as WRITE end of pipe
                close(fds[i][WRITE]); // close WRITE end of pipe
                search_and_execve(argv[i][0], argv[i]);
            }
            else { // Process reaps first child
                pg->pids[i] = pid[i];
                if (!bg) {
                    /* Parent waits for foreground job to terminate */
                    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                    reaped = 0;
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                    while(!reaped);
                } else {
                    //when there is backgrount process!
//                    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
//
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                }
            }
        }
            /* Last command */
        else if (i == pipe_count) {
            if(pid[i] == 0) { // Last child process runs last command
                Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                Signal(SIGINT, SIG_DFL);
                Signal(SIGTSTP, SIG_DFL);
                close(fds[i - 1][WRITE]); // close WRITE end of pipe
                dup2(fds[i - 1][READ], STDIN_FILENO); // duplicate STDIN as READ end of pipe
                close(fds[i - 1][READ]); // close READ end of pipe
                search_and_execve(argv[i][0], argv[i]);
            }
            else { // Process reaps last child
                close(fds[i - 1][WRITE]);
                pg->pids[i] = pid[i];
                if (!bg) {
                    /* Parent waits for foreground job to terminate */
                    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                    reaped = 0;
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                    while(!reaped);
                } else {
                    //when there is backgrount process!
//                    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                }
            }
        }
            /* commands in the middle */
        else {
            if(pid[i] == 0) { // middle child process runs command
                Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                Signal(SIGINT, SIG_DFL);
                Signal(SIGTSTP, SIG_DFL);
                close(fds[i - 1][WRITE]); // close WRITE end of pipe connected with BEFORE command
                dup2(fds[i - 1][READ], STDIN_FILENO); // duplicate STDIN as READ end of pipe
                close(fds[i - 1][READ]); // close READ end of pipe

                close(fds[i][READ]); // close READ end of pipe connected with NEXT command
                dup2(fds[i][WRITE], STDOUT_FILENO); // duplicate STDOUT as WRITE end of pipe
                close(fds[i][WRITE]); // close WRITE end of pipe

                search_and_execve(argv[i][0], argv[i]);
            }
            else { // Process reaps middle child
                close(fds[i - 1][WRITE]);
                pg->pids[i] = pid[i];
                if (!bg) {
                    /* Parent waits for foreground job to terminate */
                    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                    reaped = 0;
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                    while(!reaped);
                } else {
                    //when there is backgrount process!
//                    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                }
            }
        }
    }
}
/* $end pipe_fork_execve */

/* $begin search_and_execve */
void search_and_execve(char* filename, char** argv) {
    /* if executable file does not exist in /bin/,
       try /usr/bin  */
    if(!builtin_command(argv)) {
        if(execve(filename, argv, environ) < 0) {
            char usr[MAXBUF] = {0,};
            strcpy(usr, "/usr");
            strcat(usr, filename);
            strcpy(filename, usr);
            Execve(filename, argv, environ);
        }
    }
}
/* $end search_and_execve */

/* $begin SIGCHLD_handler */
void SIGCHLD_handler(int sig) {
    int olderrno = errno;
    pid_t pid;
    int status;
    PG* ret;

        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
    while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if((ret = PG_search(FGPGs, 0, pid, S_PID)) != NULL) { // if process in foreground
            reaped = 1;
            ret->remained--;
            if(ret->remained == 0) {
                change_PG_status(FGPGs, ret, DONE);
                PG_delete(FGPGs, ret);
                free(ret);
            }
        } else if((ret = PG_search(BGPGs, 0, pid, S_PID)) != NULL) { // if process in background
            ret->remained--;
            if (ret->remained == 0) {
                change_PG_status(BGPGs, ret, DONE);
                printf("[%d] Done\t\t%s\n", ret->job_id, ret->cmdline);
                PG_delete(BGPGs, ret);
                free(ret);
            }
        }
    }
    if (errno != ECHILD && errno != 0) {
        Sio_putl(errno);
        Sio_error("wait error");
    }
    errno = olderrno;
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
}
/* $end SIGCHLD_handler */

/* $begin SIGINT_handler */
void SIGINT_handler(int sig) {
    int olderrno = errno;
    PG* current, *next;

        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
    for (current = FGPGs->head; current != NULL; ) {
        for (int i = 0; i < current->count; ++i) {
            kill(current->pids[i], SIGINT);
        }
        change_PG_status(FGPGs, current, KILLED);
        PG_delete(FGPGs, current);
        next = current->next;
        free(current);
        current = next;
    }

    reaped = 1;

    errno = olderrno;
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
}
/* $end SIGINT_handler */

/* $begin SIGTSTP_handler */
void SIGTSTP_handler(int sig) {
    int oldererrno = errno;
    PG *current;
    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
    for (current = FGPGs->head; current != NULL; current = current->next) {
        change_PG_status(FGPGs, current, STOPPED);
        PG_delete(FGPGs, current);
        current->job_id = BGPGs->count == 0 ? 1 : BGPGs->tail->job_id + 1;
        PG_push_back(BGPGs, current);
        for (int i = 0; i < current->count; ++i) {
            kill(current->pids[i], SIGTSTP);
        }
    }
    reaped = 1;
    errno = oldererrno;
    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
}
/* $end SIGTSTP_handler */

/* $begin PGs_init */
void PGs_init(PG_list* PGs) {
    PGs->count = 0;
    PGs->head = NULL;
    PGs->tail = NULL;
}
/* $end PGs_init */

/* $begin PG_create */
PG* PG_create(int job_id, int count, char* cmd) {
    PG *node = (PG *) Malloc(sizeof(PG));
    node->job_id = job_id;
    node->count = count;
    node->pids = (pid_t *) Malloc(sizeof(pid_t) * count);

    node->status = RUNNING;
    node->remained = count;
    strcpy(node->cmdline, cmd);
    node->prev = NULL;
    node->next = NULL;

    return node;
}
/* $end PG_create */

/* $begin PG_push_back */
void PG_push_back(PG_list* PGs, PG* pg) {
    if(PGs->head == PGs->tail) {
        PGs->head = pg;
        PGs->tail = pg;
    }
    else {
        PGs->tail->next = pg;
        pg->prev = PGs->tail;
        PGs->tail = pg;
    }
    PGs->count++;
}
/* $end PG_push_back */

/* $begin PG_delete */
void PG_delete(PG_list* PGs, PG* pg) {
    PG* current;
    for (current = PGs->head; current != NULL; current = current->next) {
        if(current == pg) {
            if (current->prev != NULL && current->next != NULL) { // 중간에 있을 때
                current->prev->next = current->next;
                current->next->prev = current->prev;
            } else if (current->prev == NULL && current->next != NULL) { // head 일 때
                current->next->prev = current->prev;
                PGs->head = current->next;
            } else if (current->prev != NULL && current->next == NULL) { // tail 일 때
                current->prev->next = current->next;
                PGs->tail = current->prev;
            } else { // job이  유일
                PGs->head = PGs->tail = NULL;
            }
            PGs->count--;
            return;
        }
    }
}
/* $end PG_delete */

/* $begin PG_search */
PG* PG_search(PG_list* PGs, int job_id, pid_t pid, int option) {
    PG* current;
    switch (option) {
        case S_JOBID:
            for (current = PGs->head; current != NULL ; current = current->next) {
                if (current->job_id == job_id) {
                    return current;
                }
            }
            break;
        case S_PID:
            for (current = PGs->head; current != NULL ; current = current->next) {
                for (int i = 0; i < current->count; ++i) {
                    if(current->pids[i] == pid) {
                        return current;
                    }
                }
            }
            break;
        default:
            break;
    }

    return NULL;
}
/* $end PG_search */

/* $begin change_PG_status */
int change_PG_status(PG_list* PGs, PG* pg, int status) {
    PG* current;
    for (current = PGs->head; current != NULL; current = current->next) {
        if(current == pg) {
            current->status = status;
            return 1;
        }
    }
    return -1;
}
/* $end change_PG_status */

/* $begin PG_print */
void PG_print(PG_list* PGs) {
    PG* current;
    for (current = PGs->head; current != NULL; current = current->next) {
        switch (current->status) {
            case RUNNING:
            case KILLED:
                printf("[%d] Running\t\t%s\n", current->job_id, current->cmdline);
                break;
            case DONE:
                printf("[%d] Done\t\t%s\n", current->job_id, current->cmdline);
                break;
            case STOPPED:
                printf("[%d] Stopped\t\t%s\n", current->job_id, current->cmdline);
                break;
            default:
                break;
        }
    }
}
/* $end PG_print */
