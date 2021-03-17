#include "20150195.h"

char memory[MAX_MEMORY_SIZE] = {0,};
int last_address = 0;

int main() {
    char cmd[MAX_CMD_LEN];
    char cmd_token[4][10];
    List* list = (List*)malloc(sizeof(List));
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
            clear(cmd, cmd_token, tokens);
            continue;
        }

        if(strcmp(cmd_token[0], "quit") == 0 || strcmp(cmd_token[0], "q") == 0) {
            if(!cmd_valid_check(cmd_token, tokens, QUIT)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(list, node);
            break;
        }
        else if(strcmp(cmd_token[0], "help") == 0 || strcmp(cmd_token[0], "h") == 0) {
            if(!cmd_valid_check(cmd_token, tokens, HELP)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(list, node);

            printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\n"
                   "du[mp] [start, end]\ne[dit] address, value\n"
                   "f[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\n");

        }
        else if(strcmp(cmd_token[0], "dir") == 0 || strcmp(cmd_token[0], "d") == 0) {
            if(!cmd_valid_check(cmd_token, tokens, 2)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(list, node);

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
        else if(strcmp(cmd_token[0], "history") == 0 || strcmp(cmd_token[0], "hi") == 0) {
            if(!cmd_valid_check(cmd_token, tokens,HISTORY)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(list, node);

            Node* current;
            int j = 1;
            for (current = list->head; current != NULL; current = current->next, ++j) {
                printf("%5d %s\n", j, current->cmd_history);
            }
        }
        else if(strcmp(cmd_token[0], "dump") == 0 || strcmp(cmd_token[0], "du") == 0) {
            if(!cmd_valid_check(cmd_token, tokens,DUMP)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            if(tokens == 1) { // dump
                if(!args_check(cmd_token[1])) {
                    clear(cmd, cmd_token, tokens);
                    continue;
                }
                int i, line = last_address, end;
                if(last_address + 160 > 0xFFFFF) {
                    end = 0x100000;
                } else {
                    end = last_address + 160;
                }
                for (i = last_address; i < end; ++i) {
                    if(i%16 == 0) {
                        printf("%05X ", i);
                    }
                    printf("%02X ", memory[i]);
                    if(i%16 == 15) {
                        printf("; ");
                        for (int j = line; j < line + 16; ++j) {
                            if(memory[j] > 126 || memory[j] < 20) {
                                printf(".");
                            } else {
                                printf("%c", memory[j]);
                            }
                        }
                        printf("\n");
                        line = i;
                    }
                }
                last_address = i;
                if(last_address >= 0xFFFFF) {
                    last_address = 0;
                }
            }
            else {
                int start = (int)strtol(cmd_token[1], NULL, 16), end;
                if(tokens == 2) {
                    if(!args_check(cmd_token[1])) {
                        clear(cmd, cmd_token, tokens);
                        continue;
                    }
                    if(start + 160 > 0xFFFFF) {
                        end = 0xFFFFF;
                    } else {
                        end = start + 160;
                    }
                }
                else {
                    if(!args_check(cmd_token[1]) || !args_check(cmd_token[2])) {
                        clear(cmd, cmd_token, tokens);
                        continue;
                    }
                    end = (int)strtol(cmd_token[2], NULL, 16);

                }
                if(start > 0xFFFFF || start < 0 || end > 0xFFFFF || end < 0) {
                    printf("arguments must be in range of 0x0 to 0xFFFFF.\n");
                    clear(cmd, cmd_token, tokens);
                    continue;
                }
                if(start > end) {
                    printf("START arguments must be lower than END argument.\n");
                    clear(cmd, cmd_token, tokens);
                    continue;
                }
                int line = (start/16)*16, i;
                if(start%16 != 0) {
                    printf("%05X ", line);
                    for (int j = start%16; j > 0; --j) {
                        printf("   ");
                    }
                }
                for (i = start; i < end; ++i) {
                    if(i%16 == 0) {
                        printf("%05X ", i);
                    }
                    printf("%02X ", memory[i]);
                    if(i%16 == 15) {
                        printf("; ");
                        for (int j = line; j < line + 16; ++j) {
                            if(j < start) {
                                printf(".");
                            }
                            else if(memory[j] > 126 || memory[j] < 20) {
                                printf(".");
                            }
                            else {
                                printf("%c", memory[j]);
                            }
                        }
                        printf("\n");
                        line = i;
                    }
                    if(i%16 != 15 && i+1 == end) {
                        for (int j = 15 - (i % 16); j > 0; --j) {
                            printf("   ");
                        }
                        printf("; ");
                        for (int j = line; j < line + 16; ++j) {
                            if(j > i || memory[j] > 126 || memory[j] < 20) {
                                printf(".");
                            }
                            else {
                                printf("%c", memory[j]);
                            }
                        }
                        printf("\n");
                    }
                }
            }
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(list, node);
        }
        else if(strcmp(cmd_token[0], "edit") == 0 || strcmp(cmd_token[0], "e") == 0) {
            if(!cmd_valid_check(cmd_token, tokens, EDIT) || !args_check(cmd_token[1]) || !args_check(cmd_token[2])) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(list, node);

            int address = (int)strtol(cmd_token[1], NULL, 16), value = (int)strtol(cmd_token[2], NULL, 16);
            if(address > 0xFFFFF || address < 0) {
                printf("arguments must be in range of 0x0 to 0xFFFFF.\n");
                clear(cmd, cmd_token, tokens);
                continue;
            }
            memset(&memory[address], value, 1);
        }
        else if(strcmp(cmd_token[0], "fill") == 0 || strcmp(cmd_token[0],"f") == 0) {
            if(!cmd_valid_check(cmd_token, tokens, FILL) || !args_check(cmd_token[1]) || !args_check(cmd_token[2]) || !args_check(cmd_token[3])) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(list, node);

            int start = (int)strtol(cmd_token[1], NULL, 16), end = (int)strtol(cmd_token[2], NULL, 16),
            value = (int)strtol(cmd_token[3], NULL, 16);
            if(start > 0xFFFFF || start < 0 || end > 0xFFFFF || end < 0) {
                printf("arguments must be in range of 0x0 to 0xFFFFF.\n");
                clear(cmd, cmd_token, tokens);
                continue;
            }
            if(start > end) {
                printf("START arguments must be lower than END argument.\n");
                clear(cmd, cmd_token, tokens);
                continue;
            }
            memset(&memory[start], value, end-start+1);
        }
        else if(strcmp(cmd_token[0], "reset") == 0) {
            if(!cmd_valid_check(cmd_token, tokens, RESET)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(list, node);

            memset(memory, 0, MAX_MEMORY_SIZE);
        }
        else if(strcmp(cmd_token[0], "opcode") == 0) {
            if(!cmd_is_lower(cmd_token[1])) {
                printf("command must be lower case\n");
                continue;
            }
        }
        clear(cmd, cmd_token, tokens);
    }
}
