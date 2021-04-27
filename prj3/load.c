#include "20150195.h"

void estab_init(bucket* estab) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        estab[i].ES_head = NULL;
        estab[i].count = 0;
    }
}
void insert_ES(bucket* estab, ES_node* ES) {
    int index = hash_function(ES->name) ;
    if(estab[index].count == 0) {
        estab[index].ES_head = ES;
        estab[index].count++;
    }
    else {
        ES->next = estab[index].ES_head;
        estab[index].ES_head = ES;
        estab[index].count++;
    }
}
int search_ES(bucket* estab, char* ES_name) {
    int index = hash_function(ES_name);
    ES_node* current = estab[index].ES_head;
    for (; current != NULL ; current = current->next) {
        if (strcmp(current->name, ES_name) == 0) {
            return current->address;
        }
    }
    return FAIL;
}

ES_node* create_ESnode(char* name, int is_CSEC, int address) {
    ES_node* node = (ES_node*)malloc(sizeof(ES_node));
    node->next = NULL;
    strcpy(node->name, name);
    node->is_CSEC = is_CSEC;
    node->address = address;

    return node;
}

int load_pass1(bucket* estab, char* file1, char* file2, char* file3, int files_num) {
    char files[3][7];
    strcpy(files[0], file1);
    if(files_num >= 2) {
        strcpy(files[1], file2);
        if(files_num == 3) {
            strcpy(files[2], file3);
        }
    }

    FILE* fp;
    for (int i = 0; i < files_num; ++i) {
        fp = fopen(files[i], "r");
        if(fp == NULL) {
            printf("cannot open file %s\n", files[i]);
            return FAIL;
        }
        if(i == 0) {
            CSADDR = PROG_ADDRESS;
        }
        while(1) {
            char line[70];
            char temp[10];
            int cur_index = 1;
            fgets(line, 70, fp);
            if(feof(fp)) break;

            if(line[0] == 'H') {
                str_slice(temp, line, cur_index, 6); // H 레코드의 col 1~6까지 슬라이싱
                cur_index += 12;
                rtrim(temp);
                if(search_ES(estab, temp) != FAIL) {
                    printf("duplicate external symbol error.\n");
                    return FAIL;
                }
                ES_node* node = create_ESnode(temp, YES, CSADDR);

                str_slice(temp, line, cur_index, 6);
                CSLTH = node->length = (int)strtol(temp, NULL, 16);

                insert_ES(estab, node);

            } else if (line[0] == 'D') {
                char name[7];
                int addr = 0;
                for (; cur_index < strlen(line); cur_index += 6) {
                    if(cur_index % 12 == 1) { // ES name column
                        str_slice(temp, line, cur_index, 6);
                        rtrim(temp);
                        if(search_ES(estab, temp) != FAIL) {
                            printf("duplicate external symbol error.\n");
                            return FAIL;
                        }
                        strncpy(name, temp, strlen(temp));
                    }
                    else {
                        str_slice(temp, line, cur_index, 6);
                        addr = (int)strtol(temp, NULL, 16) + CSADDR;
                        ES_node* node = create_ESnode(name, NO, addr);
                        insert_ES(estab, node);
                        addr = 0;
                    }
                    strncpy(temp, "\0", 1);
                }
            } else break;
        }
        CSADDR += CSLTH;
    }
    return SUCCESS;
}

int load_pass2(bucket* estab, char* file1, char* file2, char* file3, int files_num) {

}
