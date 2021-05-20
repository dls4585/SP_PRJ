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
    jobs = (jobs_list *) Malloc(sizeof(jobs_list));
    jobs_list_init(jobs);

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
