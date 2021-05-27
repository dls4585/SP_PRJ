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

    Signal(SIGINT, SIGINT_handler);
    Signal(SIGTSTP, SIGTSTP_handler);
    Signal(SIGCHLD, SIGCHLD_handler);
    // default set을 prev_all에 저장
    while (1) {

        /* Read */
        fflush(stderr);
        fflush(stdout);
//        fprintf(stdout, "> ");
        printf("CSE4100-SP-P4> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        eval(cmdline);
    } 
}
/* $end shellmain */
