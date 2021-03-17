#include "20150195.h"

int main() {
    char cmd[MAX_CMD_LEN];
    char cmd_token[4][10];
    List* list = (List*)malloc(sizeof(list));
    list_init(list);
    while (1) {

        int tokens = 0;
        printf("sicsim> ");
        scanf("%[^\n]", cmd);
        trim_cmd(cmd); // 명령어 좌 우 공백 다듬기

        // 명령어와 인자 구분
        char* ptr = strtok(cmd, " ");
        while(ptr != NULL) {
            strcpy(cmd_token[tokens], ptr);
            trim_cmd(cmd_token[tokens]); // token 좌 우 공백 다듬기
            ptr = strtok(NULL, ","); // 두번째 부터는 인자로 판단 , 로 tokenize
            tokens++;
        }

        // 명령어가 소문자인지 검
        if(!cmd_is_lower(cmd_token[0])) {
            printf("command must be lower case\n");
            clear(cmd_token, tokens);
            continue;
        }

        Node* node = create_Node(cmd_token, tokens);
        list_push_back(list, node);

        if(strcmp(cmd_token[0], "quit") == 0 || strcmp(cmd_token[0], "q") == 0) {
            if(tokens > 1) {
                printf("Wrong command format, use help for command information\n");
            }
            break;
        }
        else if(strcmp(cmd_token[0], "help") == 0 || strcmp(cmd_token[0], "h") == 0) {
            if(tokens > 1) {
                printf("Wrong command format, use help for command information\n");
            }
            else {
                printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\n"
                       "du[mp] [start, end]\ne[dit] address, value\n"
                       "f[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\n");
            }
        }
        else if(strcmp(cmd_token[0], "dir") == 0 || strcmp(cmd_token[0], "d") == 0) {
            if(tokens > 1) {
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
        else if(strcmp(cmd_token[0], "history") == 0 || strcmp(cmd_token[0], "hi") == 0) {
            Node* current;
            int j = 1;
            for (current = list->head; current != NULL; current = current->next, ++j) {
                printf("%+5d %s\n", j, current->cmd_history);
            }
        }
        else if(strcmp(cmd_token[0], "dump") == 0 || strcmp(cmd_token[0], "du") == 0) {
            if(tokens == 1) { // dump
                for (int i = last_address; i < last_address + 160; ++i) {
                    if(i%16 == 0) {
                        printf("%05x ", i);
                    }
                    printf("%02x ", memory[i]);
                    if(i%16 == 15) {
                        printf("; ");
                        for (int j = last_address; j < last_address+16; ++j) {
                            if(memory[j])
                        }
                    }
                }
            }
        }

        else if(strcmp(cmd_token[0], "opcode") == 0) {
            if(!cmd_is_lower(cmd_token[1])) {
                printf("command must be lower case\n");
                continue;
            }
        }
        clear(cmd_token, tokens);
    }
}
