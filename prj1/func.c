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
            sprintf(node->cmd_history, " %s", cmd_history[j]);
        }
        else {
            sprintf(node->cmd_history, ", %s", cmd_history[j]);
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

void clear(char cmd_token[][10], int i) {
    for (int j = 0; j < i; ++j) {
        strcpy(cmd_token[j], "\0");
    }
    getchar();
}


void trim_cmd(char* cmd) {
    rtrim(cmd);
    ltrim(cmd);
}
void ltrim(char* cmd) {
    int index = 0, cmd_len = strlen(cmd);
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
    int cmd_len = strlen(cmd);
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
    int index = 0;
    for (int i = 0; i < strlen(cmd); ++i) {
        if(!islower(cmd[i])){
            return -1;
        }
    }
    return 1;
}