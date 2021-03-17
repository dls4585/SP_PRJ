#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dirent.h"
#include "sys/stat.h"
#include "errno.h"
#include "ctype.h"

#define MAX_CMD_LEN 20
#define MAX_MEMORY_SIZE 1048576

typedef struct node {
    char cmd_history[40];
    struct node* next;
} Node;

typedef struct list {
    Node* head;
    Node* tail;
} List;

void list_init(List* list);
Node* create_Node(char cmd_history[4][10], int i);
void list_push_back(List* list, Node* node);
void list_remove(List* list, Node* node);

int cmd_valid_check(char cmd_token[][10], int i);
void clear(char cmd_token[][10], int i);

void trim_cmd(char* cmd);
void ltrim(char* cmd);
void rtrim(char* cmd);
int cmd_is_lower(char* cmd);