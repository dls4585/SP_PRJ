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
 */

#include "csapp.h"
#include "myshell.h"


p_pg* create_hash(pid_t pid, pid_t pgid) {
    p_pg* node = (p_pg*) Malloc(sizeof(p_pg));
    node->pid = pid;
    node->pgid = pgid;
    return node;
}
int hash_function(pid_t pid) {
    return pid % 20;
}
void insert_hash(p_pg* ppg) {
    int hash_index = hash_function(ppg->pid);
    if(hash_table[hash_index].count == 0) {
        hash_table[hash_index].head = ppg;
        hash_table[hash_index].count++;
    }
    else {
        ppg->next = hash_table[hash_index].head;
        hash_table[hash_index].head = ppg;
        hash_table[hash_index].count++;
    }
}
int search_hash(pid_t pid) {
    int hash_index = hash_function(pid);
    p_pg* current = hash_table[hash_index].head;
    for (; current != NULL; current = current->next) {
        if(current->pid == pid) {
            return current->pgid;
        }
    }
    return -1;
}

void delete_hash(pid_t pid) {
    int hash_index = hash_function(pid);
    p_pg* current = hash_table[hash_index].head;
    for (; current != NULL; current = current->next) {
        if(current->pid == pid) {
            hash_table[hash_index].head = current->next;
            hash_table[hash_index].count--;
            free(current);
            return;
        }
    }
}

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
        if (pipe_count > 0) {
            ccount = pipe_count + 1;
            if ((pid = Fork()) == 0) {
                setpgid(0, 0);
                Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                Signal(SIGCHLD, SIGCHLD_handler);
                Signal(SIGINT, SIG_DFL);
                Signal(SIGTSTP, SIG_DFL);
                exec_pipe(argv, pipe_count, bg, cmdline);
                if(ccount == 0) exit(0);
            }
            else {
                if (!bg) {
                    /* Parent waits for foreground job to terminate */
                    int status;
                    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                    jobNode *new_fg = create_jobNode(FGjobs, RUNNING, pipe_count + 1, cmdline, pid);
                    insert_jobs(FGjobs, new_fg);
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                    Signal(SIGCHLD, SIG_DFL);
                    while(waitpid(pid, &status, 0) > 0) {
                        delete_jobs(FGjobs, 0, pid, S_PID);
                        printf("reaped!\n");
                        Signal(SIGCHLD, SIGCHLD_handler);
                    }
                }
            }
//            if((pids = exec_pipe(argv, pipe_count, bg, cmdline)) != NULL) {
//                return;
//            }
        } else {
            if ((pid = Fork()) == 0) { /* Child runs user job */
                setpgid(0, 0);
                p_pg* node = create_hash(getpid(), getpgid(pid));
                insert_hash(node);
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
                    jobNode *new_fg = create_jobNode(FGjobs, RUNNING, 1, cmdline, pid);
                    insert_jobs(FGjobs, new_fg);
                    reaped = 0;
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                    while(!reaped);
                } else {
                    //when there is backgrount process!
                    printf("%d %s", pid, cmdline);
                    Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                    jobNode *new_bg = create_jobNode(BGjobs, RUNNING, 1, cmdline, pid);
                    insert_jobs(BGjobs, new_bg);
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
        print_jobs(BGjobs);
        return 1;
    }
    if(!strcmp(argv[0], "bg")) {
        int id = (int) strtol(argv[1] + 1, NULL, 10);
        jobNode *job = search_jobs(BGjobs, id, 0, S_JOBID);
        if(job->status == STOPPED) {
            change_job_status(BGjobs, id, RUNNING);
//            kill(job->pid, SIGCONT);
            killpg(getpgid(job->pid), SIGCONT);
        }
        else if (job->status == RUNNING){
            printf("bg: job %d already in background\n", id);
        }
        else {
            printf("bg: %s: no such job\n", argv[1]);
        }
        return 1;
    }
    if(!strcmp(argv[0], "fg")) {
//        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
        int id = (int) strtol(argv[1] + 1, NULL, 10);
        jobNode *job = search_jobs(BGjobs, id, 0, S_JOBID);
        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
        if(job->status == STOPPED) {
            change_job_status(BGjobs, id, RUNNING);
        }
        delete_jobs(BGjobs, id, 0, S_JOBID);
        job->job_id = FGjobs->count == 0 ? 1 : FGjobs->tail->job_id + 1;
        insert_jobs(FGjobs, job);
        reaped = 0;
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
//        kill(job->pid, SIGCONT);
        killpg(getpgid(job->pid), SIGCONT);
        while(!reaped);
        return 1;
    }
    if(!strcmp(argv[0], "kill")) {
        int id = (int) strtol(argv[1] + 1, NULL, 10);
        jobNode *job = search_jobs(BGjobs, id, 0, S_JOBID);
        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
        delete_jobs(BGjobs, id, 0, S_JOBID);
//        reaped = 0;
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
        killpg(getpgid(job->pid), SIGKILL);
        free(job);
//        while(!reaped);
        return 1;
    }
    if(!strcmp(argv[0], "fgjobs")) {
        print_jobs(FGjobs);
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
    pid_t *pid, p;
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

    pipe_fork_execve(pipe_argv, pid, fds, pipe_count, bg, cmdline);

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
void pipe_fork_execve(char ***argv, int *pid, int **fds, int pipe_count, const int bg, char* cmdline) {
    int status;
    pid_t pgid;
    char cmd[MAXLINE];
    strcpy(cmd, cmdline);
    for (int i = 0; i <= pipe_count; ++i) {
        pid[i] = Fork();
        /* First command */
        if(i == 0) {
            if (pid[i] == 0) { // First child process runs first command
                p_pg* node = create_hash(getpid(), getpgid(getpid()));
                insert_hash(node);
                Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                Signal(SIGINT, SIG_DFL);
                Signal(SIGTSTP, SIG_DFL);

                close(fds[i][READ]); // close READ end of pipe
                dup2(fds[i][WRITE], STDOUT_FILENO); // duplicate STDOUT as WRITE end of pipe
                close(fds[i][WRITE]); // close WRITE end of pipe
                search_and_execve(argv[i][0], argv[i]);
                fprintf(stderr, "Failed to execute %s\n", argv[i][0]);

            }
            else {
                Sigprocmask(SIG_BLOCK, &mask_all, NULL);

                if (!bg) {
                    /* Parent waits for foreground job to terminate */
//                    jobNode *new_fg = create_jobNode(FGjobs, RUNNING, pipe_count + 1, cmdline, getpgid(pid[i]));
//                    insert_jobs(FGjobs, new_fg);
                    reaped = 0;
                Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                    while (!reaped);
                } else {
                    //when there is backgrount process!
                    printf("%d\n", pid[0]);

                    Sigprocmask(SIG_BLOCK, &mask_all, NULL);;
                    jobNode *new_bg = create_jobNode(BGjobs, RUNNING, pipe_count + 1, cmdline, getpgid(pid[i]));
                    insert_jobs(BGjobs, new_bg);
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                }
                ccount--;
            }
        }
            /* Last command */
        else if (i == pipe_count) {
            if(pid[i] == 0) { // Last child process runs last command
                p_pg* node = create_hash(getpid(), pid[0]);
                insert_hash(node);

                Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                Signal(SIGINT, SIG_DFL);
                Signal(SIGTSTP, SIG_DFL);

                close(fds[i - 1][WRITE]); // close WRITE end of pipe
                dup2(fds[i - 1][READ], STDIN_FILENO); // duplicate STDIN as READ end of pipe
                close(fds[i - 1][READ]); // close READ end of pipe
                search_and_execve(argv[i][0], argv[i]);
                fprintf(stderr, "Failed to execute %s\n", argv[i][0]);
            }
            else { // Process reaps last child
                close(fds[i - 1][WRITE]);
                Sigprocmask(SIG_BLOCK, &mask_all, NULL);
                if(!bg) {
                    /* Parent waits for foreground job to terminate */
                    reaped = 0;
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                    while(!reaped);
                } else {
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                }
                ccount--;
            }
        }
            /* commands in the middle */
        else {
            if(pid[i] == 0) { // middle child process runs command
                p_pg* node = create_hash(pid[i], getpgid(pid[i]));
                insert_hash(node);
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
                fprintf(stderr, "Failed to execute %s\n", argv[i][0]);

            }
            else { // Process reaps middle child
                close(fds[i - 1][WRITE]);
                Sigprocmask(SIG_BLOCK, &mask_all, NULL);

                if(!bg) {
                    /* Parent waits for foreground job to terminate */
                    reaped = 0;
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                    while(!reaped);
                } else {
                    Sigprocmask(SIG_SETMASK, &prev_one, NULL);
                }
                ccount--;
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
    pid_t pid, pgid;
    int status;
    jobNode* ret;

    while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
        pgid = search_hash(pid);
        if((ret = search_jobs(FGjobs, 0, pgid, S_PID)) != NULL) { // if process in foreground
            ret->count--;
            if(ret->count == 0) {
                change_job_status(FGjobs, ret->job_id, DONE);
                delete_jobs(FGjobs, 0, ret->pid, S_PID);
                free(ret);
//                p_reaped = 1;
            }
        } else if((ret = search_jobs(BGjobs, 0, pgid, S_PID)) != NULL) { // if process in background
            ret->count--;
            if(ret->count == 0) {
                change_job_status(BGjobs, ret->job_id, DONE);
                printf("[%d] Done\t\t%s", ret->job_id, ret->cmdline);
                delete_jobs(BGjobs, 0, ret->pid, S_PID);
                free(ret);
//                p_reaped = 1;
            }
        }
        delete_hash(pid);
        reaped = 1;
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
    }
    if (errno != ECHILD && errno != 0) {
        Sio_putl(errno);
        Sio_error("wait error");
    }
    errno = olderrno;
}
/* $end SIGCHLD_handler */

///* $begin BG_SIGCHLD_handler */
//void BG_SIGCHLD_handler(int sig) {
//    int olderrno = errno;
//    pid_t pid;
//    int status;
//
//    jobNode* ret;
//
//    while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
//        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
//        bg_ccount--;
//        bg_reaped = 1;
//        if ((ret = search_jobs(BGjobs, 0, pid, S_PID)) != NULL) {
//            change_job_status(BGjobs, ret->job_id, DONE);
//            printf("[%d] Done\t\t%s", ret->job_id, ret->cmdline);
//            delete_jobs(BGjobs, 0, ret->pid, S_PID);
//            free(ret);
//        }
//        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
//        if(bg_ccount == 0) {
//            Signal(SIGCHLD, FG_SIGCHLD_handler);
//        }
//    }
//    if (errno != ECHILD && errno != 0) {
//        Sio_putl(errno);
//        Sio_error("wait error");
//    }
//    errno = olderrno;
//}
///* $end BG_SIGCHLD_handler */
//
///* $begin FG_SIGCHLD_handler */
//void FG_SIGCHLD_handler(int sig) {
//    int olderrno = errno;
//    pid_t pid;
//    int status;
//    jobNode* ret;
//
//    while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
//        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
//        reaped = 1;
//        ccount--;
//        if ((ret = search_jobs(FGjobs, 0, pid, S_PID)) != NULL) {
//            change_job_status(FGjobs, ret->job_id, DONE);
//            delete_jobs(FGjobs, 0, ret->pid, S_PID);
//            free(ret);
//        }
//        if(ccount == 0 && bg_reaped == 0) {
//            Signal(SIGCHLD, BG_SIGCHLD_handler);
//        }
//        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
//    }
//    if (errno != ECHILD && errno != 0) {
//        Sio_putl(errno);
//        Sio_error("wait error");
//    }
//    errno = olderrno;
//}
///* $end FG_SIGCHLD_handler */

/* $begin SIGINT_handler */
void SIGINT_handler(int sig) {
    int olderrno = errno;
    int status;
    jobNode* current;

    for (current = FGjobs->head; current != NULL; current = current->next) {

        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
        change_job_status(FGjobs, current->job_id, KILLED);
        delete_jobs(FGjobs, 0, current->pid, S_PID);
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
        killpg(getpgid(current->pid), SIGINT);
    }
    errno = olderrno;
}
/* $end SIGINT_handler */

/* $begin SIGTSTP_handler */
void SIGTSTP_handler(int sig) {
    int oldererrno = errno;
    jobNode *current;
    for (current = FGjobs->head; current != NULL; current = current->next) {
        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
        change_job_status(FGjobs, current->job_id, STOPPED);
        delete_jobs(FGjobs, 0, current->pid, S_PID);
        current->job_id = BGjobs->count == 0 ? 1 : BGjobs->tail->job_id+1;
        insert_jobs(BGjobs, current);
        Sigprocmask(SIG_SETMASK, &prev_one, NULL);
        killpg(getpgid(current->pid), SIGTSTP);
    }
    errno = oldererrno;
}
/* $end SIGTSTP_handler */

/* $begin create_jobNode */
jobNode* create_jobNode(jobs_list* jobsList, int status, int count, char* cmdline, pid_t pid) {
    jobNode *node = (jobNode *) Malloc(sizeof(jobNode));
    node->job_id = jobsList->count == 0 ? 1 : jobsList->tail->job_id + 1;
    node->status = status;
    node->count = count;
    strcpy(node->cmdline, cmdline);
    node->pid = pid;
    node->prev = NULL;
    node->next = NULL;

    return node;
}
/* $end create_jobNode */

/* $begin jobs_list_init */
void jobs_list_init(jobs_list* jobsList) {
    jobsList->head = NULL;
    jobsList->tail = NULL;
    jobsList->count = 0;
}
/* $end jobs_list_init */

/* $begin insert_jobs */
void insert_jobs(jobs_list* jobsList, jobNode* node) {
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
jobNode *search_jobs(jobs_list *jobsList, int id, pid_t pid, int option) {
    jobNode* current;
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
jobNode* delete_jobs(jobs_list* jobsList, int id, pid_t pid, int option) {
    jobNode* current;
    switch (option) {
        case S_JOBID:
            for (current = jobsList->head; current != NULL ; current = current->next) {
                if(current->job_id == id) {
                    jobNode* ret = current;
                    if (current->prev != NULL && current->next != NULL) { // 중간에 있을 때
                        current->prev->next = current->next;
                        current->next->prev = current->prev;
                    }
                    else if (current->prev == NULL && current->next != NULL) { // head 일 때
                        current->next->prev = current->prev;
                        jobsList->head = current->next;
                    }
                    else if (current->prev != NULL && current->next == NULL) { // tail 일 때
                        current->prev->next = current->next;
                        jobsList->tail = current->prev;
                    }
                    else { // job이  유일
                        jobsList->head = jobsList->tail = NULL;
                    }
                    jobsList->count--;
                    return ret;
                }
            }
            break;
        case S_PID:
            for (current = jobsList->head; current != NULL ; current = current->next) {
                if(current->pid == pid) {
                    jobNode* ret = current;
                    if (current->prev != NULL && current->next != NULL) { // 중간에 있을 때
                        current->prev->next = current->next;
                        current->next->prev = current->prev;
                    }
                    else if (current->prev == NULL && current->next != NULL) { // head 일 때
                        current->next->prev = current->prev;
                        jobsList->head = current->next;
                    }
                    else if (current->prev != NULL && current->next == NULL) { // tail 일 때
                        current->prev->next = current->next;
                        jobsList->tail = current->prev;
                    }
                    else { // job이  유일
                        jobsList->head = jobsList->tail = NULL;
                    }
                    jobsList->count--;
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
jobNode* change_job_status(jobs_list* jobsList, int id, int status) {
    jobNode* current;
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
    jobNode *current;
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
