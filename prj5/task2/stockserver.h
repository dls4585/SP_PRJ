//
// Created by 백인찬 on 2021/06/04.
//

#include "csapp.h"

struct item {
    int ID;
    int left_stock;
    int price;
    int readcnt; /* Semaphore  */
    sem_t mutex;
    sem_t w;
    struct item* left;
    struct item* right;
};

struct AVLtree {
    struct item* root;
    int count;
};

struct AVLtree* items;


void construct_stock_tree();
void update_file();

void init_tree();
struct item* insert_item(struct item* node, struct item* new_item);
struct item* search_item(struct item* node, int ID);
int change_stock(int ID, int new_amount);
struct item* create_item(int id, int left_stock, int price);

void in_traverse(FILE* fp, struct item* node, char *buf);
void rio_traverse(struct item* node, int connfd, char buf[]);

struct item* LL(struct item* parent);
struct item* RR(struct item* parent);
struct item* LR(struct item* parent);
struct item* RL(struct item* parent);

int get_height(struct item* node);
int get_height_diff(struct item* node);
struct item* make_balance(struct item* node);


int action(int connfd);
void parseline(char buf[], char cpy[][MAXLINE]);
int exec_cmd(char cpy[][MAXLINE], char buf[]);

void *thread_func(void *arg);

