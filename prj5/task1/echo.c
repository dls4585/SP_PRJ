/*
 * echo - read and echo text lines until client closes connection
 */
/* $begin echo */
#include "csapp.h"

void echo(int connfd) 
{
    int n; 
    char buf[MAXLINE]; 
    rio_t rio;

    Rio_readinitb(&rio, connfd);
//    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
//	    printf("server received %d bytes\n", n);
//	    Rio_writen(connfd, buf, n);
//    }

    if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { // read message and echo then return not to be blocking others
        printf("server received %d bytes\n", n);
        Rio_writen(connfd, buf, n);
    }
}
/* $end echo */

