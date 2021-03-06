/* 
 * echoserveri.c - An iterative action server
 */ 
/* $begin echoserverimain */
#include "csapp.h"
#include "stockserver.h"

int main(int argc, char **argv) 
{

    init_tree();
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
                        if(action(i) < 0) {
                            update_file();
                            Close(i);
                            FD_CLR(i, &in);
                        }
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
        items->root=insert_item(items->root, new_item);
        items->count++;
    }

}
/* $end construct_stock_tree */


/* $begin AVL Rotate function */

struct item* LL(struct item* parent) {
    struct item* child = parent->left;
    parent->left = child->right;
    child->right = parent;
    return child;
}

struct item* RR(struct item* parent) {
    struct item* child = parent->right;
    parent->right = child->left;
    child->left = parent;
    return child;
}

struct item* LR(struct item* parent) {
    struct item* child = parent->left;
    parent->left = RR(child);
    return LL(parent);
}

struct item* RL(struct item* parent) {
    struct item* child = parent->right;
    parent->right = LL(child);
    return RR(parent);
}

int get_height(struct item* node) {
    int height, h_left, h_right;
    height = 0;
    if (node) {
        h_left = get_height(node->left);
        h_right = get_height(node->right);
        height = (h_left > h_right ? h_left : h_right) + 1;
    }
    return height;
}

int get_height_diff(struct item* node) {
    if (!node) {
        return 0;
    }
    else {
        return get_height(node->left) - get_height(node->right);
    }
}

struct item* make_balance(struct item* node) {
    int height = get_height_diff(node);
    if (height > 1) { // left sub tree is deeper than right
        if (get_height_diff(node->left) > 0) {
            node = LL(node);
        }
        else {
            node = LR(node);
        }
    }
    else if(height < -1) {
        if (get_height_diff(node->right) < 0) {
            node = RR(node);
        }
        else {
            node = RL(node);
        }
    }
    return node;
}
/* $end AVL Rotate function */

/* $begin init_tree */
void init_tree() {
    items = (struct AVLtree*) Malloc(sizeof(struct AVLtree));
    items->root = NULL;
    items->count = 0;
}
/* $end init_tree */

/* $begin insert_item */
struct item* insert_item(struct item* node, struct item* new_item) {
    if(items->count == 0) {
        items->root = new_item;
        items->count++;
        return new_item;
    }
    if(node == NULL) {
        node = new_item;
    }
    else if (new_item->ID < node->ID) {
        node->left = insert_item(node->left, new_item);
        node = make_balance(node);
    }
    else if (new_item->ID > node->ID) {
        node->right = insert_item(node->right, new_item);
        node = make_balance(node);
    }
    else {
        printf("insert error\n");
        exit(0);
    }
    return node;
}
/* $end insert_item */

/* $begin search_item */
struct item* search_item(struct item* node, int ID) {
    if (node == NULL) {
        return NULL;
    }
    if(ID < node->ID) {
        return search_item(node->left, ID);
    }
    else if (ID > node->ID) {
        return search_item(node->right, ID);
    }
    else {
        return node;
    }
}
/* $end search_item */

/* $begin change_stock */
int change_stock(int ID, int new_amount) {
    struct item* target = search_item(items->root, ID);
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


void update_file() {
    FILE* fp = fopen("stock.txt", "w");
    if (fp == NULL) {
        printf("cannot open stock.txt\n");
        exit(1);
    }
    in_traverse(fp, items->root, NULL);

    fclose(fp);
}

void in_traverse(FILE* fp, struct item* node, char *buf) {
    if(node) {
        in_traverse(fp, node->left, buf);
        if(fp == NULL) {
            sprintf(buf, "%s%d %d %d\t", buf, node->ID, node->left_stock, node->price);
        }
        else {
            fprintf(fp, "%d %d %d\n", node->ID, node->left_stock, node->price);
        }
        in_traverse(fp, node->right, buf);
    }
}
