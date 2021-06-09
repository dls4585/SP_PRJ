/*
 * echo - read and action text lines until client closes connection
 */
/* $begin action */
#include "csapp.h"
#include "stockserver.h"

#define CYN   "\x1B[36m"

void action(int connfd)
{
    int n; 
    char buf[MAXLINE], cpy[3][MAXLINE], write_buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);

    if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { // read message and action then return not to be blocking others
        printf("server received %d bytes\n", n);
        parseline(buf, cpy);
        exec_cmd(cpy, write_buf);
        write_buf[strlen(write_buf)] = '\n';
        Fputs(write_buf, stdout);
        Rio_writen(connfd, write_buf, strlen(write_buf));
    }
}
/* $end action */

void parseline(char buf[], char cpy[][MAXLINE]) {
    char *tmp, *ptr;
    char temp[MAXLINE];
    int i = 0;
    strcpy(temp, buf);

    tmp = temp;
    if(!strcmp(buf, "")) return;
    tmp[strlen(tmp) - 1] = '\0';
    while((ptr = strsep(&tmp, " ")) != NULL) {
        strcpy(cpy[i++], ptr);
    }
    if (i < 1) {
        printf("parse error\n");
        return;
    }
}

void exec_cmd(char cpy[][MAXLINE], char buf[]) {
    if(!strcmp(cpy[0], "show")) {
        update_file();
        pre_traverse(NULL, items->root, buf);
//        rio_traverse(items->root, connfd, buf);
    }
    else if (!strcmp(cpy[0], "buy")) {
        int stock_id = (int) strtol(cpy[1], NULL, 10);
        int amount = (int) strtol(cpy[2], NULL, 10);
        struct item *target = search_item(stock_id);
        int new_amount = target->left_stock - amount;
        if(new_amount < 0) {
            sprintf(buf, "Not enough left stock\n");
            return;
        }
        else {
            if (change_stock(stock_id, new_amount)) {
                strcpy(buf, "change error\n");
                exit(1);
            }
            else {
                sprintf(buf, "[buy] success\n");
            }
        }
    }
    else if (!strcmp(cpy[0], "sell")) {
        int stock_id = (int) strtol(cpy[1], NULL, 10);
        int amount = (int) strtol(cpy[2], NULL, 10);
        struct item *target = search_item(stock_id);
        int new_amount = target->left_stock + amount;
        if (change_stock(stock_id, new_amount)) {
            sprintf(buf, "change error\n");
            exit(1);
        }
        else {
            sprintf(buf, "[sell] success\n");
        }
    }
    else if (!strcmp(cpy[0], "exit")) {
        return;
    }
}

