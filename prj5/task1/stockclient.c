/*
 * echoclient.c - An action client
 */
/* $begin echoclientmain */
#include "csapp.h"

int main(int argc, char **argv)
{
    int clientfd, n;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));
//        while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        Rio_readnb(&rio, buf, MAXLINE);
        Fputs(buf, stdout);
//        }
        Fputs("end", stdout);
//        Rio_readnb(&rio, buf, MAXLINE);
//        Fputs(buf, stdout);
    }
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}
/* $end echoclientmain */
