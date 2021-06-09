/* 
 * echoserveri.c - An iterative action server
 */ 
/* $begin echoserverimain */
#include "csapp.h"
#include "stockserver.h"

int main(int argc, char **argv) 
{

    init_tree();
    init_queue();
    construct_stock_tree();


    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    char client_hostname[MAXLINE], client_port[MAXLINE];
    fd_set in, out, excp, temp;
    struct timeval nulltime;
    int fd_max, num;
    if (argc != 2) {
	    fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(0);
    }

    listenfd = Open_listenfd(argv[1]);
    nulltime.tv_sec = 0;
    nulltime.tv_usec = 0;

    FD_ZERO(&in);
    FD_ZERO(&out);
    FD_ZERO(&excp);
    FD_SET(listenfd, &in);
    fd_max = listenfd;

    while (1) {
        temp = in;
        if ((num = Select(fd_max + 1, &temp, &out, &excp, &nulltime)) == -1) break;

        if(num == 0) continue;

        for (int i = 0; i < fd_max + 1; ++i) {
            if(FD_ISSET(i, &in)) { // check if we need to check this fd "i"
                if(FD_ISSET(i, &temp)) { // check if we have any readable request from fd "i"
                    if(i == listenfd) { // connection request
                        clientlen = sizeof(struct sockaddr_storage);
                        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
                        FD_SET(connfd, &in); // set connfd as what is needed to be checked
                        if(fd_max < connfd) {
                            fd_max = connfd;
                        }
                        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
                                    client_port, MAXLINE, 0);
                        printf("Connected to (%s, %s)\n", client_hostname, client_port);
                    }
                    else { // get message from fd "i"
                        action(i);
//                        Close(i);
                    }
                }
            }
        }
    }
    close(listenfd);
    exit(0);
}
/* $end echoserverimain */

/* $begin construct_stock_tree */
void construct_stock_tree() {
    FILE* f = fopen("stock.txt", "r");
    if (f == NULL) {
        fprintf(stderr, "cannot open stock.txt\n");
        exit(1);
    }
    int id, left_stock, price;
    while(fscanf(f, "%d %d %d", &id, &left_stock, &price) != EOF) {
        struct item *new_item = create_item(id, left_stock, price);
        insert_queue(create_itemQ(new_item));
        insert_item(new_item);
    }

}
/* $end construct_stock_tree */


/* $begin init_tree */
void init_tree() {
    items = (struct itemBTree*) Malloc(sizeof(struct itemBTree));
    items->root = NULL;
    items->count = 0;
}
/* $end init_tree */

/* $begin insert_item */
void insert_item(struct item* new_item) {
    if(items->root == NULL) {
        items->root = new_item;
        insert_queue(create_itemQ(new_item));
    }
    else {
        while(1){
            if (Q->front->node->left == NULL) {
                Q->front->node->left = new_item;
                break;
            }
            else if (Q->front->node->right == NULL) {
                Q->front->node->right = new_item;
                pop_queue();
                break;
            }
            else {
                pop_queue();
            }
        }
        insert_queue(create_itemQ(new_item));
    }
    items->count++;
}
/* $end insert_item */

/* $begin search_item */
struct item* search_item(int ID) {
    if (items->root->ID == ID) {
        return items->root;
    }
    int currentID = ID, n = 0;
    int *track = (int *) Malloc(sizeof(int) * (items->count / 2));
    track[n++] = currentID;
    while (currentID != 1) {
        track[n++] = currentID / 2;
        currentID /= 2;
    }
    n--;
    struct item* cur = items->root;
    while(1) {
        if (track[n - 1] % 2 == 1) {
            cur = cur->right;
        }
        else {
            cur = cur->left;
        }
        n--;
        if(cur->ID == ID) {
            free(track);
            return cur;
        }
        if(n < 0) break;
    }
    free(track);
    return NULL;
}
/* $end search_item */

/* $begin change_stock */
int change_stock(int ID, int new_amount) {
    struct item* target = search_item(ID);
    if(target == NULL) return -1;
    target->left_stock = new_amount;
    return 1;
}
/* $end change_stock */

/* $begin create_item */
struct item* create_item(int id, int left_stock, int price) {
    struct item* node = (struct item*) Malloc(sizeof(struct item));
    node->ID = id;
    node->left_stock = left_stock;
    node->price = price;
    node->left = NULL;
    node->right = NULL;

    return node;
}
/* $end create_item */


void init_queue() {
    Q = (struct queue*) Malloc(sizeof(struct queue));
    Q->front = NULL;
    Q->rear = NULL;
}

void insert_queue(struct itemQ* new_item) {
    if (Q->front == NULL) {
        Q->front = Q->rear = new_item;
    }
    else {
        Q->rear->next = new_item;
        Q->rear = new_item;
    }
}
struct itemQ* create_itemQ(struct item* new_item) {
    struct itemQ* node = (struct itemQ*) Malloc(sizeof(struct itemQ));
    node->node = new_item;
    node->next = NULL;
    return node;
}

struct item* pop_queue() {
    struct itemQ* ret = Q->front;
    if(ret == NULL) return NULL;
    else {
        Q->front = Q->front->next;
        return ret->node;
    }
}

void update_file() {
    FILE* fp = fopen("stock.txt", "w");
    if (fp == NULL) {
        printf("cannot open stock.txt\n");
        exit(1);
    }
    pre_traverse(fp, items->root, NULL);

    fclose(fp);
}

void pre_traverse(FILE* fp, struct item* node, char buf[]) {
    if(node) {
        if(fp == NULL) {
            sprintf(buf, "%s%d %d %d\n", buf, node->ID, node->left_stock, node->price);
        }
        else {
            fprintf(fp, "%d %d %d\n", node->ID, node->left_stock, node->price);
        }
        pre_traverse(fp, node->left, buf);
        pre_traverse(fp, node->right, buf);
    }
}

void rio_traverse(struct item* node, int connfd, char buf[]) {
    if (node) {
        sprintf(buf, "%s %d %d %d\n", buf, node->ID, node->left_stock, node->price);
        rio_traverse(node->left, connfd, buf);
        rio_traverse(node->right, connfd, buf);
        buf[strlen(buf) - 1] = '\0';
        Rio_writen(connfd, buf, strlen(buf));
    }
}