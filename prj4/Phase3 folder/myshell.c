/*
 * myshell.c - main function for myshell
 *
 * Updated 2021/05/16 - 백인찬
 * - arranged functions into separate source files.
 */

/* $begin shellmain */
#include "csapp.h"
#include "myshell.h"

PG* pgs[MAXARGS] = {NULL,};
int pgs_index = 0;

int main() 
{
    char cmdline[MAXLINE]; /* Command line */


    nullfd = open("/dev/null", O_WRONLY);

    FGPGs = (PG_list *) Malloc(sizeof(PG_list));
    BGPGs = (PG_list *) Malloc(sizeof(PG_list));

    PGs_init(FGPGs);
    PGs_init(BGPGs);

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
    close(nullfd);
}
/* $end shellmain */
