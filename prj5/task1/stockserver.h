//
// Created by 백인찬 on 2021/06/04.
//

#include "csapp.h"

struct item {
    int ID;
    int left_stock;
    int price;
    int readcnt;
    sem_t mutex;
    sem_t w;
    struct item* left;
    struct item* right;
};

struct itemBTree {
    struct item* root;
    int count;
};

struct itemQ {
    struct item* node;
    struct itemQ* next;
};

struct queue {
    struct itemQ* front;
    struct itemQ* rear;
};

struct itemBTree* items;

struct queue* Q;

void construct_stock_tree();
void update_file();

void init_tree();
void insert_item(struct item* new_item);
struct item* search_item(int ID);
int change_stock(int ID, int new_amount);
struct item* create_item(int id, int left_stock, int price);

void pre_traverse(FILE* fp, struct item* node, char buf[]);
void rio_traverse(struct item* node, int connfd, char buf[]);


void init_queue();
struct itemQ* create_itemQ(struct item* new_item);
void insert_queue(struct itemQ* new_item);
struct item* pop_queue();

void action(int connfd);
void parseline(char buf[], char cpy[][MAXLINE]);
void exec_cmd(char cpy[][MAXLINE], char buf[]);

