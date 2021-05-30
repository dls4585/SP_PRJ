/*
 * myshell.c - main function for myshell
 *
 * Updated 2021/05/16 - 백인찬
 * - arranged functions into separate source files.
 */

/* $begin shellmain */
#include "csapp.h"
#include "myshell.h"

PG* done_PGs[MAXARGS] = {NULL,}; // array for Process Group which is DONE
int pgs_index = 0; // index for array above

int main() 
{
    char cmdline[MAXLINE]; /* Command line */


    nullfd = open("/dev/null", O_WRONLY); // fd for writing to /dev/null

    /* init data structures */
    FGPGs = (PG_list *) Malloc(sizeof(PG_list));
    BGPGs = (PG_list *) Malloc(sizeof(PG_list));

    PGs_init(FGPGs);
    PGs_init(BGPGs);

    /* init Sigsets */
    Sigfillset(&mask_all);
    Sigemptyset(&mask_one);
    Sigaddset(&mask_one, SIGCHLD);

    /* set handlers for SIGINT, SIGTSTP, SIGCHLD on shell(parent) process */
    Signal(SIGINT, SIGINT_handler);
    Signal(SIGTSTP, SIGTSTP_handler);
    Signal(SIGCHLD, SIGCHLD_handler);
    while (1) {
        fflush(stderr);
        fflush(stdout);

        /* Read */
        printf("CSE4100-SP-P4> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        eval(cmdline);
    }
    close(nullfd);
}
/* $end shellmain */
