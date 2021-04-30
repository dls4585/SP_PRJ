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
    char files[3][30];
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
            printf("cannot open file %s in load pass 1.\n", files[i]);
            return FAIL;
        }
        if(i == 0) {
            CSADDR = PROG_ADDRESS;
        }
        while(1) {
            char line[70];
            char temp[7];
            int cur_index = 1;
            fgets(line, 70, fp);
            line[strlen(line)-1] = '\0';
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
                int addr = 0;
                for (; cur_index < strlen(line); cur_index += 6) {
                    char name[7];
                    if(cur_index % 12 == 1) { // ES name column
                        str_slice(temp, line, cur_index, 6);
                        rtrim(temp);
                        if(search_ES(estab, temp) != FAIL) {
                            printf("duplicate external symbol error.\n");
                            return FAIL;
                        }
                    }
                    else {
                        strcpy(name, temp);
                        str_slice(temp, line, cur_index, 6);
                        addr = (int)strtol(temp, NULL, 16) + CSADDR;
                        ES_node* node = create_ESnode(name, NO, addr);
                        insert_ES(estab, node);
                        addr = 0;
                        temp[0] = '\0';
                    }
                }
            } else break;
        }
        CSADDR += CSLTH;
    }
    return SUCCESS;
}

int load_pass2(bucket* estab, char* file1, char* file2, char* file3, int files_num) {
    char files[3][30];
    strcpy(files[0], file1);
    if(files_num >= 2) {
        strcpy(files[1], file2);
        if(files_num == 3) {
            strcpy(files[2], file3);
        }
    }

    FILE* fp;
    int ref_num[3][6] = {0,};
    for (int i = 0; i < files_num; ++i) {
        fp = fopen(files[i], "r");
        if(fp == NULL) {
            printf("cannot open file %s in load pass 2.\n", files[i]);
            return FAIL;
        }


        while(1) {
            char line[70];
            char temp[7];
            int cur_index = 1;
            fgets(line, 70, fp);
            line[strlen(line)-1] = '\0';
            if(feof(fp)) break;

            if(line[0] == 'D' || line[0] == '.') continue;
            else if(line[0] == 'H') {
                str_slice(temp, line, 1, 6);
                rtrim(temp);
                if((CSADDR = search_ES(estab, temp)) == FAIL) {
                    printf("cannot search prog name in estab.\n");
                    return FAIL;
                }
                ref_num[i][1] = CSADDR;
            }
            else if(line[0] == 'R') {
                int index;
                while(cur_index < strlen(line)) {
                    if(cur_index % 8 == 1) { // ref_num 을 가리키는 column
                        str_slice(temp, line, cur_index, 2);
                        index = (int)strtol(temp, NULL, 16);
                        cur_index+=2;
                    }
                    else if (cur_index % 8 == 3) { // extref symbol을 나타내는 column
                        str_slice(temp, line, cur_index, 6);
                        rtrim(temp);
                        if((ref_num[i][index] = (search_ES(estab, temp))) == FAIL) {
                            printf("undefined external symbol.\n");
                            return FAIL;
                        }
                        cur_index += 6;
                    }
                }
            }
            else if(line[0] == 'T') {
                int T_startaddr, T_size, current_addr;
                unsigned int byte_value;

                str_slice(temp, line, cur_index, 6);
                cur_index += 6;
                T_startaddr = (int)strtol(temp, NULL, 16) + CSADDR;
                current_addr = T_startaddr;

                str_slice(temp, line, cur_index, 2);
                T_size = (int)strtol(temp, NULL, 16);
                cur_index += 2;

                for (int j = 0; j < T_size; ++j) {
                    str_slice(temp, line, cur_index, 2);
                    byte_value = (unsigned int)strtol(temp, NULL, 16);
                    memset(&memory[current_addr], byte_value, 1);
                    current_addr++;
                    cur_index+=2;
                }
            }
            else if(line[0] == 'M') {
                modify(temp, line, cur_index, ref_num, i);
            }
            else break;
        }
    }


    return SUCCESS;
}

void modify(char* temp, char* line, int cur_index, int ref_num[][6], int i) {
    int mod_target_addr, half_bytes_num, index;
    int origin_value = 0, mod_value = 0;
    str_slice(temp, line, cur_index, 6);
    mod_target_addr = (int)strtol(temp, NULL, 16) + CSADDR;
    cur_index += 6;

    str_slice(temp, line, cur_index, 2);
    half_bytes_num = (int)strtol(temp, NULL, 16);
    cur_index+= 2;

    str_slice(temp, line, cur_index+1, 2);
    index = (int)strtol(temp, NULL, 16);

    if(line[cur_index] == '+') {
        mod_value += ref_num[i][index];

    } else {
        mod_value -= ref_num[i][index];
    }

    if(half_bytes_num % 2 == 1) {
        int temp_value = memory[mod_target_addr];
        int or_value = 0;

        for (int j = 0; j <= half_bytes_num / 2; ++j) {
            origin_value = origin_value << 8;
            origin_value = (origin_value | memory[mod_target_addr+j]);
        }

        temp_value = temp_value & 0xF0;
        temp_value = temp_value << (8*(half_bytes_num/2));

        for (int j = 0; j < half_bytes_num; ++j) {
            or_value = or_value << 4;
            or_value = or_value | 0xF;
        }

        origin_value = origin_value & or_value;
        origin_value += mod_value;
        origin_value = temp_value | origin_value;
    }
    else {
        for (int j = 0; j < half_bytes_num / 2; ++j) {
            origin_value = origin_value << 8;
            origin_value = (origin_value | memory[mod_target_addr + j]);
        }

        origin_value += mod_value;
   }

    unsigned char* src = (unsigned char*)malloc(sizeof(unsigned char)*(half_bytes_num+1)/2);
    for (int j = ((half_bytes_num + 1) / 2) - 1; j >= 0; --j) {
        src[j] = cut_by_byte_from_last(origin_value);
        origin_value = origin_value >> 8;
    }

    memcpy(&memory[mod_target_addr], src, (half_bytes_num+1)/2);
}

unsigned int cut_by_byte_from_last(unsigned int number) {
    unsigned int temp = number & 0xFF;
    return temp;
}

void run(int* BP_list, int BP_count) {
    
}