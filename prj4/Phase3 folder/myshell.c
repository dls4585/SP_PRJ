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

    while (1) {
    	/* Read */
        fflush(stderr);
        fflush(stdout);
//        fprintf(stdout, "> ");
        Sio_puts("> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        /* Evaluate */
        eval(cmdline);
    } 
}
/* $end shellmain */
