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

int run(int* BP_list, int BP_count, bucket* optab) {

    while (PC != PROG_LENGTH) { // 수정
        int opcode, ni_bits, xbpe;
        int current_byte;
        char temp[10];
        int x,b,p,e;
        int TA;
        int j_flag = NO;
        int sign_bit = 1;

        if(BP_flag == NO && check_BP(BP_list, BP_count) == YES) {
            print_registers();
            BP_flag = YES;
            return PC;
        }
        BP_flag = NO;

        current_byte = memory[PC];
        opcode = current_byte >> 2;
        opcode <<= 2;
        ni_bits = current_byte & 0x03;

        search_by_opcode(optab, opcode, temp);
        if(strcmp(temp, "FAIL") == 0) {
            printf("cannot disassemble opcode.\n");
            return FAIL;
        }
        hash_node* opnode = search_opcode(optab, temp);
        if(opnode->format[0] == 2) {
            nextPC = PC+2;
            current_byte = memory[PC+1];
            TA = current_byte;
        }else {
            current_byte = memory[PC+1];
            xbpe = current_byte >> 4;
            x = xbpe >> 3;
            b = (xbpe >> 2) & 0x01;
            p = (xbpe >> 1) & 0x01;
            e = xbpe & 0x01;

            TA = (current_byte & 0x0F) << 8;

            if(e == 1) {
                nextPC = PC+4;
                TA <<= 8;
                current_byte = memory[PC+2];
                TA = TA | (current_byte << 8);
                current_byte = memory[PC+3];
                TA = TA | current_byte;
            } else {
                nextPC = PC+3;
                current_byte = memory[PC+2];
                TA = TA | current_byte;
            }
            sign_bit = TA >> 11;
            if(sign_bit == 1) {
                TA |= (int)0xFFFFF000;
            }


            if (x == 1) {
                TA += X;
            }

            if(b == 1 && p == 0) { // base relative
                TA += B;
            } else if(b == 0 && p == 1) { // pc relative
                TA += nextPC;
            }
        }

        int r1, r2;
        switch (opcode) {
            case 0x14: // STL
                store(&L, TA, ni_bits);
                break;
            case 0x68: // LDB
                load(&B, TA, ni_bits);
                break;
            case 0x48: // JSUB
                L = nextPC;
                j_flag = YES;
                jump(TA, ni_bits);
                break;
            case 0x00: // LDA
                load(&A, TA, ni_bits);
                break;
            case 0x28: // COMP
                if(ni_bits == 3) { // simple
                    if(A > memory[TA]) {
                        CC = '>';
                    } else if (A == memory[TA]) {
                        CC = '=';
                    } else {
                        CC = '<';
                    }
                } else if (ni_bits == 2) { // indirect
                    if(A > memory[memory[TA]]) {
                        CC = '>';
                    } else if (A == memory[TA]) {
                        CC = '=';
                    } else {
                        CC = '<';
                    }
                } else if (ni_bits == 1) { // immediate
                    if(A > TA) {
                        CC = '>';
                    } else if (A == TA) {
                        CC = '=';
                    } else {
                        CC = '<';
                    }
                }
                break;
            case 0x30: // JEQ
                if(CC == '=') {
                    jump(TA, ni_bits);
                    j_flag = YES;
                }
                break;
            case 0x3C: // J
                jump(TA, ni_bits);
                j_flag = YES;
                break;
            case 0x0C: // STA
                store(&A, TA, ni_bits);
                break;
            case 0xB4: // CLEAR
                TA = TA >> 4;
                switch (TA) {
                    case 0:
                        A = 0;
                        break;
                    case 1:
                        X = 0;
                        break;
                    case 2:
                        L = 0;
                        break;
                    case 3:
                        B = 0;
                        break;
                    case 4:
                        S = 0;
                        break;
                    case 5:
                        T = 0;
                        break;
                    default:
                        break;
                }
                break;
            case 0x74: // LDT
                load(&T, TA, ni_bits);
                break;
            case 0xE0: // TD
                CC = '<';
                break;
            case 0xD8: // RD
                CC = '=';
                break;
            case 0x54: // STCH
                if(ni_bits == 3) { // simple
                    memory[TA] = (A & 0xFF);
                } else if (ni_bits == 2) { // indirect
                    memory[memory[TA]] = (A & 0xFF);
                }
                break;
            case 0xB8: // TIXR
                TA = TA >> 4;
                X++;
                switch (TA) {
                    case 0:
                        TA = A;
                        break;
                    case 1:
                        TA = X;
                        break;
                    case 2:
                        TA = L;
                        break;
                    case 3:
                        TA = B;
                        break;
                    case 4:
                        TA = S;
                        break;
                    case 5:
                        TA = T;
                        break;
                    default:
                        break;
                }
                if(X > T) {
                    CC = '>';
                } else if (X == T) {
                    CC = '=';
                } else {
                    CC = '<';
                }
                break;
            case 0x38: // JLT
                if(CC == '<') {
                    jump(TA, ni_bits);
                    j_flag = YES;
                }
                break;
            case 0x10: // STX
                store(&X, TA, ni_bits);
                break;
            case 0x4C: // RSUB
                nextPC = L;
                break;
            case 0x50: // LDCH
                A >>= 8;
                A <<= 8;
                if(ni_bits == 3) { // simple
                    A |= memory[TA];
                } else if (ni_bits == 2) { // indirect
                    A |= memory[memory[TA]];
                } else if (ni_bits == 1) { // immediate
                    A |= TA;
                }
                break;
            case 0xA0: // COMPR
                r1 = TA & 0xF0;
                r2 = TA & 0x0F;
                switch (r1) {
                    case 0:
                        r1 = A;
                        break;
                    case 1:
                        r1 = X;
                        break;
                    case 2:
                        r1 = L;
                        break;
                    case 3:
                        r1 = B;
                        break;
                    case 4:
                        r1 = S;
                        break;
                    case 5:
                        r1 = T;
                        break;
                    default:
                        break;
                }
                switch (r2) {
                    case 0:
                        r2 = A;
                        break;
                    case 1:
                        r2 = X;
                        break;
                    case 2:
                        r2 = L;
                        break;
                    case 3:
                        r2 = B;
                        break;
                    case 4:
                        r2 = S;
                        break;
                    case 5:
                        r2 = T;
                        break;
                    default:
                        break;
                }
                if(r1 > r2) {
                    CC = '>';
                } else if (r1 == r2) {
                    CC = '=';
                } else {
                    CC = '<';
                }
                break;
            case 0xDC: // WD
            default:
                break;
        }
        if(j_flag == NO) {
            PC = nextPC;
        }
    }

    return SUCCESS;
}

void search_by_opcode(bucket* optab, int opcode, char temp[10]) {
    hash_node* current;
    for (int i = 0; i < HASH_SIZE; ++i) {
        for (current = optab[i].head; current != NULL ; current = current->next) {
            if(current->opcode == opcode) {
                strcpy(temp, current->mnemonic);
                return;
            }
        }
    }
    strcpy(temp, "FAIL");
}

void store(const int* reg, int TA, int ni_bits) {
    int first_byte = *reg >> (8*2);
    int second_byte = (*reg >> 8) & 0xFF;
    int third_byte = *reg & 0xFF;
    if(ni_bits == 3) { // simple
        memset(&memory[TA], first_byte, 1);
        memset(&memory[TA+1], second_byte, 1);
        memset(&memory[TA+2], third_byte, 1);
    } else if(ni_bits == 2) { // indirect
        memset(&memory[memory[TA]], first_byte, 1);
        memset(&memory[memory[TA+1]], second_byte, 1);
        memset(&memory[memory[TA+2]], third_byte, 1);
    }
}

void load(int* reg, int TA, int ni_bits) {
    if(ni_bits == 3) { // simple
        *reg = memory[TA];
        *reg <<= 8;
        *reg |= memory[TA+1];
        *reg <<= 8;
        *reg |= memory[TA+2];
    } else if (ni_bits == 2) { // indirect
        *reg = memory[memory[TA]];
        *reg <<= 8;
        *reg |= memory[memory[TA+1]];
        *reg <<= 8;
        *reg |= memory[memory[TA+2]];
    } else { // immediate
        *reg = TA;
    }
}

int check_BP(int* BP_list, int BP_count) {
    for (int i = 0; i < BP_count; ++i) {
        if(PC == BP_list[i]) {
            return YES;
        }
    }
    return NO;
}

void print_registers() {
    printf("A : %06X\tX : %06X\n", A, X);
    printf("L : %06X\t\bPC : %06X\n", L, PC);
    printf("B : %06X\tS : %06X\n", B, S);
    printf("T : %06X\n", T);
}

void jump(int TA, int ni_bits) {
    if(ni_bits == 3) { // simple
        PC = TA;
    } else if (ni_bits == 2) { // indirect
        PC = memory[TA];
        PC <<= 8;
        PC |= memory[TA+1];
        PC <<= 8;
        PC |= memory[TA+2];
    }
}