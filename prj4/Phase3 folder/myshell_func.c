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
 * - implemented backgrond process for piped command with &.
 */

#include "csapp.h"
#include "myshell.h"

sigset_t mask, prev_mask;
int ccount = 0;
int bg_ccount = 0;
handler_t* oldhandler;

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
        if (pipe_count > 0) {
            if((pids = exec_pipe(argv, pipe_count)) != NULL) {
                if (!bg) {
                    /* Parent waits for foreground job to terminate */
                    ccount = pipe_count + 1;
                    while(ccount > 0) {
                        if(Wait(NULL) > 0) {
                            ccount--;
                        }
                    }
                } else {
                    //when there is backgrount process!
                    printf("%d\n", pids[pipe_count]);
                    BGNode *new_bg = create_BGNode(jobs, RUNNING, cmdline, pids[0]);

                    insert_jobs(jobs, new_bg);

                    bg_ccount += (pipe_count + 1);
                    oldhandler = Signal(SIGCHLD, SIGCHLD_handler);

                    Sigemptyset(&mask);
                    Sigaddset(&mask, SIGINT);
                    Sigaddset(&mask, SIGSTOP);
                    Sigaddset(&mask, SIGCONT);
                    Sigprocmask(SIG_BLOCK, &mask, &prev_mask);

                    return;
                }
            }
        } else {
            if ((pid = Fork()) == 0) { /* Child runs user job */
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
                    Wait(NULL);
                } else {
                    //when there is backgrount process!
                    printf("%d %s", pid, cmdline);
                    BGNode *new_bg = create_BGNode(jobs, RUNNING, cmdline, pid);

                    insert_jobs(jobs, new_bg);

                    bg_ccount++;
                    oldhandler = Signal(SIGCHLD, SIGCHLD_handler);

                    Sigemptyset(&mask);
                    Sigaddset(&mask, SIGINT);
                    Sigaddset(&mask, SIGSTOP);
                    Sigaddset(&mask, SIGCONT);
                    Sigprocmask(SIG_BLOCK, &mask, &prev_mask);

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
        print_jobs(jobs);
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
int* exec_pipe(char** argv, const int pipe_count) {
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
            return NULL;
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
    return pid;
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
//                Waitpid(pid[i], &status, 0);
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
//                Waitpid(pid[i], &status, 0);
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
//                Waitpid(pid[i], &status, 0);
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
    BGNode* ret;
    if((pid = Waitpid(-1, &status, WNOHANG)) > 0) {
        bg_ccount--;
        if ((ret = search_jobs(jobs, 0, pid, S_PID)) != NULL) {
            change_job_status(jobs, ret->job_id, DONE);
            printf("[%d] Done\t\t%s", ret->job_id, ret->cmdline);
            delete_jobs(jobs, 0, ret->pid, S_PID);
            free(ret);
        }
    }
    if (errno == ECHILD) {
        Sio_putl(errno);
        Sio_error("wait error");
    }
    if(bg_ccount == 0) {
        Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
        Signal(SIGCHLD, oldhandler);
    }
    errno = olderrno;
}
/* $end SIGCHLD_handler */



/* $begin create_BGNode */
BGNode* create_BGNode(jobs_list* jobsList, int status, char* cmdline, pid_t pid) {
    BGNode *node = (BGNode *) Malloc(sizeof(BGNode));
    node->job_id = jobsList->count == 0 ? 1 : jobsList->tail->job_id + 1;
    node->status = status;
    node->cmdline = cmdline;
    node->pid = pid;
    node->prev = NULL;
    node->next = NULL;

    return node;
}
/* $end create_BGNode */

/* $begin jobs_list_init */
void jobs_list_init(jobs_list* jobsList) {
    jobsList->head = NULL;
    jobsList->tail = NULL;
    jobsList->count = 0;
}
/* $end jobs_list_init */

/* $begin insert_jobs */
void insert_jobs(jobs_list* jobsList, BGNode* node) {
    if(jobsList->count == 0) {
        jobsList->head = node;
        jobsList->tail = node;
        jobsList->count++;
    }
    else {
        jobsList->tail->next = node;
        node->prev = jobsList->tail;
        jobsList->tail = node;
        jobsList->count++;
    }
}
/* $end insert_jobs */

/* $begin search_jobs */
BGNode *search_jobs(jobs_list *jobsList, int id, pid_t pid, int option) {
    BGNode* current;
    switch (option) {
        case S_JOBID:
            for (current = jobsList->head; current != NULL ; current = current->next) {
                if (current->job_id == id) {
                    return current;
                }
            }
            break;
        case S_PID:
            for (current = jobsList->head; current != NULL ; current = current->next) {
                if (current->pid == pid) {
                    return current;
                }
            }
            break;
        default:
            break;
    }

    return NULL;
}
/* $end search_jobs */

/* $begin delete_jobs */
BGNode* delete_jobs(jobs_list* jobsList, int id, pid_t pid, int option) {
    BGNode* current;
    switch (option) {
        case S_JOBID:
            for (current = jobsList->head; current != NULL ; current = current->next) {
                if(current->job_id == id) {
                    BGNode* ret = current;
                    if (current->prev != NULL) {
                        current->prev->next = current->next;
                    }
                    else {
                        current->next->prev = NULL;
                    }
                    if (current->next != NULL) {
                        current->next->prev = current->prev;
                    }
                    else {
                        current->prev->next = NULL;
                    }
                    jobs->count--;
                    return ret;
                }
            }
            break;
        case S_PID:
            for (current = jobsList->head; current != NULL ; current = current->next) {
                if(current->pid == pid) {
                    BGNode* ret = current;
                    if (current->prev != NULL && current->next != NULL) {
                        current->prev->next = current->next;
                        current->next->prev = current->prev;
                    }
                    else if (current->prev == NULL && current->next != NULL) {
                        current->next->prev = current->prev;
                    }
                    else if (current->prev != NULL && current->next == NULL) {
                        current->prev->next = current->next;
                    }
                    jobs->count--;
                    return ret;
                }
            }
            break;
        default:
            break;
    }

    return NULL;
}
/* $end delete_jobs */

/* $begin change_job_status */
BGNode* change_job_status(jobs_list* jobsList, int id, int status) {
    BGNode* current;
    for (current = jobsList->head; current != NULL ; current = current->next) {
        if(current->job_id == id) {
            current->status = status;
            return current;
        }
    }
    return NULL;
}
/* $end change_job_status */

/* $begin print_jobs */
void print_jobs(jobs_list *jobsList) {
    BGNode *current;
    for (current = jobsList->head; current != NULL; current = current->next) {
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
/* $end print_jobs */
