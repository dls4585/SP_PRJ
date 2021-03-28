/* 포함되는 파일들 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dirent.h"
#include "sys/stat.h"
#include "errno.h"
#include "ctype.h"

/* 정의되는 상수 */
#define MAX_CMD_LEN 100
#define MAX_MEMORY_SIZE 1048576
#define HASH_SIZE 20

#define QUIT 0
#define HELP 1
#define DIRECTORY 2
#define HISTORY 3
#define DUMP 4
#define EDIT 5
#define FILL 6
#define RESET 7
#define MNEMONIC 8
#define OPLIST 9

/* 정의되는 구조체 */
typedef struct node {
    char cmd_history[MAX_CMD_LEN];
    struct node* next;
} Node;

typedef struct list {
    Node* head;
    Node* tail;
} List;

typedef struct hash_node {
    char mnemonic[10];
    int opcode;
    int format[2];
    struct hash_node* next;
} hash_node;

typedef struct bucket {
    struct hash_node* head;
    int count;
} bucket;

/* 함수목록 */
void list_init(List* list);
Node* create_Node(char cmd_history[4][MAX_CMD_LEN], int i);
void list_push_back(List* list, Node* node);

int cmd_valid_check(int i, int cmd_case);
int args_check(char* args);
void clear(char* cmd, char cmd_token[][MAX_CMD_LEN], int i);

void trim_cmd(char* cmd);
void ltrim(char* cmd);
void rtrim(char* cmd);
int is_lower(char* cmd);
int is_upper(char* cmd);

int hash_init(bucket* hashtable);
void insert_hash(bucket* hashtable, hash_node* hash);
int hash_function(char* mnemonic);
int hash_search(bucket* hashtable, char* mnemonic);

void replaceTab(char* string);