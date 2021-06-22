#include "csapp.h"
#include <time.h>

#define MAX_CLIENT 100
#define ORDER_PER_CLIENT 10
#define STOCK_NUM 5
#define BUY_SELL_MAX 10

int main(int argc, char **argv)
{
    pid_t pids[MAX_CLIENT];
    int runprocess = 0, status, i;

    int clientfd, num_client;
    char *host, *port, buf[MAXLINE], tmp[3];
    rio_t rio;

    if (argc != 4) {
        fprintf(stderr, "usage: %s <host> <port> <client#>\n", argv[0]);
        exit(0);
    }

    host = argv[1];
    port = argv[2];
    num_client = atoi(argv[3]);

    struct timespec start, finish;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &start);

/*	fork for each client process	*/
    while(runprocess < num_client){
        //wait(&state);
        pids[runprocess] = fork();

        if(pids[runprocess] < 0)
            return -1;
            /*	child process		*/
        else if(pids[runprocess] == 0){
            printf("child %ld\n", (long)getpid());

            clientfd = Open_clientfd(host, port);
            Rio_readinitb(&rio, clientfd);
            srand((unsigned int) getpid());

            for(i=0;i<ORDER_PER_CLIENT;i++){
//				int option = rand() % 3; // clients can request for all type
//				int option = rand() % 2 + 1; // clients request only for buy and sell
                int option = 0; // clients request only for show

                if(option == 0){//show
                    strcpy(buf, "show\n");
                }
                else if(option == 1){//buy
                    int list_num = rand() % STOCK_NUM + 1;
                    int num_to_buy = rand() % BUY_SELL_MAX + 1;//1~10

                    strcpy(buf, "buy ");
                    sprintf(tmp, "%d", list_num);
                    strcat(buf, tmp);
                    strcat(buf, " ");
                    sprintf(tmp, "%d", num_to_buy);
                    strcat(buf, tmp);
                    strcat(buf, "\n");
                }
                else if(option == 2){//sell
                    int list_num = rand() % STOCK_NUM + 1;
                    int num_to_sell = rand() % BUY_SELL_MAX + 1;//1~10

                    strcpy(buf, "sell ");
                    sprintf(tmp, "%d", list_num);
                    strcat(buf, tmp);
                    strcat(buf, " ");
                    sprintf(tmp, "%d", num_to_sell);
                    strcat(buf, tmp);
                    strcat(buf, "\n");
                }
                //strcpy(buf, "buy 1 2\n");
//                printf("%d : ", clientfd);
//                Fputs(buf, stdout);
                Rio_writen(clientfd, buf, strlen(buf));
                Rio_readlineb(&rio, buf, MAXLINE);
                if(!strncmp(buf, "no", 2)) break;
                char* ptr;
                while((ptr = strchr(buf, '\t')) != NULL) {
                    *ptr = '\n';
                }
                Fputs(buf, stdout);
                memset(&buf, 0, MAXLINE);
//				usleep(1000000);
            }
            Close(clientfd);
            exit(0);
        }
        /*	parten process		*/
        /*else{
            for(i=0;i<num_client;i++){
                waitpid(pids[i], &status, 0);
            }
        }*/
        runprocess++;
    }

    for(i=0;i<num_client;i++){
        waitpid(pids[i], &status, 0);
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("elapsed time : %lf\n", elapsed);

    /*clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));
        Rio_readlineb(&rio, buf, MAXLINE);
        Fputs(buf, stdout);
    }

    Close(clientfd); //line:netp:echoclient:close
    exit(0);*/

    return 0;
}
