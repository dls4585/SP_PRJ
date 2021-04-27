/* 포함되는 파일들 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dirent.h"
#include "sys/stat.h"
#include "errno.h"
#include "ctype.h"
#include <math.h>

/* 정의되는 상수 */
#define MAX_CMD_LEN 100
#define MAX_MEMORY_SIZE 1048576
#define HASH_SIZE 20

#define SUCCESS 1
#define FAIL -1

#define NONE -1

#define YES 1
#define NO 0

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
#define TYPE 10
#define ASSEMBLE 11
#define SYMBOL 12
#define PROGADDR 13
#define LOADER 14
#define BP 15
#define RUN 16

/* 전역변수 */
extern char memory[MAX_MEMORY_SIZE];
extern int last_address;
extern int PC;
extern int CSADDR;
extern int PROG_ADDRESS;
extern int CSLTH;

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

typedef struct symbol_node {
    char name[30];
    int LOCCTR;
    struct symbol_node* next;
} symbol_node;

typedef struct bucket {
    struct hash_node* head;
    struct symbol_node* s_head;
    struct ES_node* ES_head;
    int count;
} bucket;

typedef struct line_node {
    int line;
    int LOCCTR;
    char symbol[30];
    char mnemonic[100];
    char operand[2][30];
    int obj_code;
    int mod_bit;
    struct line_node* prev;
    struct line_node* next;
} line_node;

typedef struct line_list {
    line_node* head;
    line_node* tail;
} line_list;

typedef struct ES_node {
    char name[7];
    int address;
    int is_CSEC;
    int length;
    struct ES_node* next;
} ES_node;

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

int hash_function(char* field);
int optab_init(bucket* hashtable);
void insert_opcode(bucket* hashtable, hash_node* hash);
hash_node* search_opcode(bucket* hashtable, char* mnemonic);

void replaceTab(char* string);
void str_slice(char *dest, const char *src, int start, int size);

void symtab_init(bucket* symtab);
void insert_symbol(bucket* symtab, symbol_node* symbol);
int search_symbol(bucket* symtab, char* symbol_name);

int pass1(char* filename, bucket* optab, bucket* symtab, line_list* linelist, int* lines, int* prgm_len, int* error_flag);
int pass2(char* filename, bucket* optab, bucket* symtab, line_list* linelist, int* prgm_len);

void line_list_init(line_list* list);
void line_list_push_back(line_list* list, line_node* node);
line_node* create_line_node(int line, int LOCCTR, char symbol[], char mnemonic[], char operand1[], char operand2[]);

int write_lst_file(line_list* list, char* filename);
int write_obj_file(line_list* list, char* filename, const int* prgm_len);

int load_pass1(bucket* estab, char* file1, char* file2, char* file3, int files_num);
int load_pass2(bucket* estab, char* file1, char* file2, char* file3, int files_num);

void estab_init(bucket* estab);
void insert_ES(bucket* estab, ES_node* ES);
int search_ES(bucket* estab, char* ES_name);
ES_node* create_ESnode(char* name, int is_CSEC, int address);

