/*
 * echo - read and action text lines until client closes connection
 */
/* $begin action */
#include "csapp.h"
#include "stockserver.h"

#define CYN   "\x1B[36m"

int action(int connfd)
{
    int n;
    char buf[MAXLINE], cpy[3][MAXLINE], write_buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    memset(&write_buf, 0, MAXLINE);
    if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { // read message and action then return not to be blocking others
        printf("server received %d bytes\n", n);
        parseline(buf, cpy);
        if (exec_cmd(cpy, write_buf) < 0) {
            return -1;
        }
        write_buf[strlen(write_buf)] = '\0';
        Rio_writen(connfd, write_buf, strlen(write_buf));
    }
    return 0;
}
/* $end action */

void parseline(char buf[], char cpy[][MAXLINE]) {
    char *tmp, *ptr;
    char temp[MAXLINE];
    int i = 0;
    strcpy(temp, buf);

    tmp = temp;
    if(!strcmp(buf, "\n")) return;
    tmp[strlen(tmp) - 1] = '\0';
    while((ptr = strsep(&tmp, " ")) != NULL) {
        strcpy(cpy[i++], ptr);
    }
    if (i < 1) {
        printf("parse error\n");
        return;
    }
}

int exec_cmd(char cpy[][MAXLINE], char buf[]) {
    if(!strcmp(cpy[0], "\n")) return 0;
    char temp[3][MAXLINE];
    for (int i = 0; i < 3; ++i) {
        strcpy(temp[i], cpy[i]);
    }
    if(!strcmp(temp[0], "show")) {
        update_file();
        in_traverse(NULL, items->root, buf);
    }
    else if (!strcmp(temp[0], "buy")) {
        int stock_id = (int) strtol(temp[1], NULL, 10);
        int amount = (int) strtol(temp[2], NULL, 10);
        struct item *target = search_item(items->root, stock_id);
        int new_amount = target->left_stock - amount;
        if(new_amount < 0) {
            strcpy(buf, "Not enough left stock\n");
        }
        else {
            if (!change_stock(stock_id, new_amount)) {
                printf(buf, "change error\n");
                exit(1);
            }
            else {
                strcpy(buf, "[buy] success\n");
            }
        }
    }
    else if (!strcmp(temp[0], "sell")) {
        int stock_id = (int) strtol(temp[1], NULL, 10);
        int amount = (int) strtol(temp[2], NULL, 10);
        struct item *target = search_item(items->root, stock_id);
        int new_amount = target->left_stock + amount;
        if (!change_stock(stock_id, new_amount)) {
            printf(buf, "change error\n");
            exit(1);
        }
        else {
            strcpy(buf, "[sell] success\n");
        }
    }
    else if (!strcmp(cpy[0], "exit")) {
        return -1;
    }
    return 0;
}

