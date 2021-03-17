#include "20150195.h"


void list_init(List* list) {
    list->head = NULL;
    list->tail = NULL;
}

Node* create_Node(char cmd_history[4][10], int i) {
    Node* node = (Node*)malloc(sizeof(Node));
    for (int j = 0; j < i; ++j) {
        if(j == 0) {
            sprintf(node->cmd_history, "%s", cmd_history[j]);
        }
        else if(j == 1) {
            sprintf(node->cmd_history, "%s %s", node->cmd_history, cmd_history[j]);
        }
        else {
            sprintf(node->cmd_history, "%s, %s", node->cmd_history, cmd_history[j]);
        }
    }
    node->next = NULL;
    return node;
}

void list_push_back(List* list, Node* node) {
    if(list->head == NULL && list->tail == NULL) {
        list->head = list->tail = node;
    }
    else {
        list->tail->next = node;
        list->tail = node;
    }
}

int cmd_valid_check(char cmd_token[][10], int tokens, int cmd_case) {
    switch (cmd_case) {
        case 0: // quit
        case 1: // help
        case 2: // dir
        case 3: // history
        case 7: // reset
        case 9: // opcodelist
            if(tokens > 1) {
                printf("Wrong command format, use help for command information\n");
                return 0;
            }
            break;
        case 4: // dump
            if(tokens > 4) {
                printf("Wrong command format, use help for command information\n");
                return 0;
            }
            break;
        case 5: // edit
            if(tokens != 3) {
                printf("Wrong command format, use help for command information\n");
                return 0;
            }
            break;
        case 6: // fill
            if(tokens != 4) {
                printf("Wrong command format, use help for command information\n");
                return 0;
            }
            break;
        case 8: // opcode mnemonic
            if(tokens != 2) {
                printf("Wrong command format, use help for command information\n");
                return 0;
            }
            break;
        default:
            return 1;
    }
    return 1;
}

int args_check(char* args) {
    if(strstr(args, " ")!= NULL || strstr(args, "\t") != NULL) {
        printf("Wrong arguments, arguments must be in range of 0x0 to 0xFFFFF\n");
        return 0;
    }
    for (int i = 0; i < strlen(args); ++i) {
        if((args[i] >= 48 && args[i] <= 57) || (args[i] >= 65 && args[i] <= 70) || (args[i] >= 97 && args[i] <= 102)) {
            continue;
        } else {
            printf("Wrong arguments, arguments must be in range of 0x0 to 0xFFFFF\n");
            return 0;
        }
    }
    return 1;
}

void clear(char* cmd, char cmd_token[][10], int i) {
    for (int j = 0; j < i; ++j) {
        strcpy(cmd_token[j], "\0");
    }
    strcpy(cmd, "\0");
    getchar();
}


void trim_cmd(char* cmd) {
    rtrim(cmd);
    ltrim(cmd);
}
void ltrim(char* cmd) {
    int index = 0, cmd_len = (int)strlen(cmd);
    char trimmed[MAX_CMD_LEN];

    strcpy(trimmed, cmd);
    for (int i = 0; i < cmd_len; ++i) {
        if(trimmed[i] == ' ' || trimmed[i] == '\t') {
            index++;
        }
        else {
            break;
        }
    }
    strcpy(cmd, trimmed+index);
}
void rtrim(char* cmd) {
    int cmd_len = (int)strlen(cmd);
    char trimmed[MAX_CMD_LEN];

    strcpy(trimmed, cmd);
    for (int i = cmd_len-1; i >= 0; --i) {
        if(trimmed[i] == ' ' || trimmed[i] == '\t' || trimmed[i] == '\n') {
            continue;
        }
        else {
            trimmed[i+1] = '\0';
            break;
        }
    }
    strcpy(cmd, trimmed);
}

int cmd_is_lower(char* cmd) {
    for (int i = 0; i < strlen(cmd); ++i) {
        if(!islower(cmd[i])){
            return 0;
        }
    }
    return 1;
}