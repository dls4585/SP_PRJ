#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dirent.h"
#include "sys/stat.h"
#include "errno.h"
#include "20150195.h"

void clear(char cmd_token[][10], int i);

int main() {
    char cmd[20];
    char cmd_token[4][10];
    while (1) {
        printf("sicsim> ");
        scanf("%[^\n]", cmd);
        char* ptr = strtok(cmd, " ");
        int i = 0;
        while(ptr != NULL) {
            strcpy(cmd_token[i], ptr);
            ptr = strtok(NULL, " ");
            i++;
        }
        if(strcmp(cmd_token[0], "quit") == 0 || strcmp(cmd_token[0], "q") == 0) {
            if(i > 1) {
                printf("Wrong command format, use help for command information\n");
            }
            break;
        }
        else if(strcmp(cmd_token[0], "help") == 0 || strcmp(cmd_token[0], "h") == 0) {
            if(i > 1) {
                printf("Wrong command format, use help for command information\n");
            }
            else {
                printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\n"
                       "du[mp] [start, end]\ne[dit] address, value\n"
                       "f[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\n");
            }
        }
        else if(strcmp(cmd_token[0], "dir") == 0 || strcmp(cmd_token[0], "d") == 0) {
            if(i > 1) {
                printf("Wrong command format, use help for command information\n");
            }
            else {
                DIR *dp = opendir(".");
                struct dirent *dent;
                struct stat info;

                while (dent = readdir(dp)) {
                    char path[1024];
                    sprintf(path, "./%s", dent->d_name);
                    int stats = stat(path, &info);
                    if(stats == -1) {
                        printf("%s", strerror(errno));
                    }
                    if(S_ISDIR(info.st_mode)) {
                        printf("%s/\n", dent->d_name);
                    }
                    else if(info.st_mode & S_IXUSR) {
                        printf("%s*\n", dent->d_name);
                    }
                    else {
                        printf("%s\n", dent->d_name);
                    }
                }
            }
        }
        clear(cmd_token, i);
    }
}

void clear(char cmd_token[][10], int i) {
    for (int j = 0; j < i; ++j) {
        strcpy(cmd_token[j], "\0");
    }
    getchar();
}