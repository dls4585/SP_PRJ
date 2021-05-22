/*
 * myshell.c - main function for myshell
 *
 * Updated 2021/05/16 - 백인찬
 * - arranged functions into separate source files.
 */

/* $begin shellmain */
#include "csapp.h"
#include "myshell.h"

int main() 
{
    char cmdline[MAXLINE]; /* Command line */
    BGjobs = (jobs_list *) Malloc(sizeof(jobs_list));
    FGjobs = (jobs_list *) Malloc(sizeof(jobs_list));
    jobs_list_init(BGjobs);
    jobs_list_init(FGjobs);

    Sigfillset(&mask_all);
    Sigemptyset(&mask_one);
    Sigaddset(&mask_one, SIGCHLD);
    oldhandler = Signal(SIGCHLD, FG_SIGCHLD_handler);

    Signal(SIGINT, SIGINT_handler);
    Signal(SIGTSTP, SIGTSTP_handler);
    Sigprocmask(SIG_SETMASK, &mask_all, &prev_all);
    Sigprocmask(SIG_SETMASK, &prev_all, NULL);
    while (1) {
        /* Read */
        fflush(stderr);
        fflush(stdout);
//        fprintf(stdout, "> ");
        printf("> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        eval(cmdline);
    } 
}
/* $end shellmain */
