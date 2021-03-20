#include "20150195.h"


void list_init(List* list) {
    list->head = NULL;
    list->tail = NULL;
}

Node* create_Node(char cmd_history[4][MAX_CMD_LEN], int i) {
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

int cmd_valid_check(int tokens, int cmd_case) {
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
            if(tokens > 3) {
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

void clear(char* cmd, char cmd_token[][MAX_CMD_LEN], int i) {
    for (int j = 0; j < i; ++j) {
        strcpy(cmd_token[j], "\0");
    }
    strcpy(cmd, "\0");
//    getchar();
    rewind(stdin);
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

int is_lower(char* cmd) {
    for (int i = 0; i < strlen(cmd); ++i) {
        if(!islower(cmd[i])){
            return 0;
        }
    }
    return 1;
}

int is_upper(char* cmd) {
    for (int i = 0; i < strlen(cmd); ++i) {
        if (!isupper(cmd[i])) {
            return 0;
        }
    }
    return 1;
}

int hash_init(bucket* hashtable) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        hashtable[i].count = 0;
        hashtable[i].head = NULL;
    }
    FILE* fp = fopen("opcode.txt", "r");
    if(fp == NULL) {
        printf("opcode.txt file does not exist\n");
        return 0;
    }
    while(1) {
        char line[50];
        char line_tokens[3][10];
        int tokens = 0;
        hash_node* hash = (hash_node*)malloc(sizeof(hash_node));
        hash->format[0] = 0;
        hash->format[1] = 0;
        hash->next = NULL;

        fgets(line, 50, fp);
        if(feof(fp)) break;

        int len = (int)strlen(line);
        line[len-1] = line[len-2] = '\0';

        char* ptr = strtok(line, "\t");
        while(ptr != NULL) {
            trim_cmd(ptr);
            strcpy(line_tokens[tokens], ptr);
            ptr = strtok(NULL, "\t");
            tokens++;
        }
        if(tokens != 3) {
            printf("file read is going wrong\n");
            return 0;
        }

        hash->opcode = (int)strtol(line_tokens[0], NULL, 16);
        strcpy(hash->mnemonic, line_tokens[1]);

        if(strlen(line_tokens[2]) < 2) {
            hash->format[0] = (int)strtol(line_tokens[2], NULL, 10);
        }
        else {
            ptr = strtok(line_tokens[2], "/");
            for (int i = 0; i < 2; ++i) {
                hash->format[i] = (int)strtol(ptr, NULL, 10);
                ptr = strtok(NULL, "/");
            }
        }

        insert_hash(hashtable, hash);
    }
    fclose(fp);
    return 1;
}

void insert_hash(bucket* hashtable, hash_node* hash) {
    int hash_index = hash_function(hash->mnemonic);
    if(hashtable[hash_index].count == 0) {
        hashtable[hash_index].head = hash;
        hashtable[hash_index].count++;
    }
    else {
        hash->next = hashtable[hash_index].head;
        hashtable[hash_index].head = hash;
        hashtable[hash_index].count++;
    }
}

int hash_function(char* mnemonic) {
    int len = (int)strlen(mnemonic), ret = 0;
    for (int i = 0; i < len; ++i) {
        ret += (mnemonic[i] * (i + 1));
    }
    ret %= HASH_SIZE;
    return ret;
}

int hash_search(bucket* hashtable, char* mnemonic) {
    int hash_index = hash_function(mnemonic);
    hash_node* current = hashtable[hash_index].head;
    for (; current != NULL; current = current->next) {
        if(strcmp(current->mnemonic, mnemonic) == 0)
            return current->opcode;
    }
    return -1;
}

void removeTab(char* string) {
    char* find;
    while(1) {
        find = strchr(string, '\t');
        if(!find) return;
        strncpy(find, " ",1);
    }
}