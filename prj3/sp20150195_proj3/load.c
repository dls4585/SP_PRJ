#include "20150195.h"

/*
 * 목적 : ESTAB을 초기화한다.
 * 리턴값 : 없음
 */
void estab_init(bucket* estab) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        estab[i].ES_head = NULL;
        estab[i].count = 0;
    }
}

/*
 * 목적 : 새로운 ES를 ESTAB에 삽입한다.
 * 리턴값 : 없음
 */
void insert_ES(bucket* estab, ES_node* ES) {
    int index = hash_function(ES->name);
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

/*
 * 목적 : ESTAB에서 ES_name에 해당하는 ES_node를 찾는다.
 * 리턴값 : 성공 시 - 해당 ES_node의 주소를 반환한다.
 *        실패 시 - -1
 */
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

/*
 * 목적 : 새로운 ES_node를 만든다.
 * 리턴값 : 새로 만든 ES_node를 반환한다.
 */
ES_node* create_ESnode(char* name, int is_CSEC, int address) {
    ES_node* node = (ES_node*)malloc(sizeof(ES_node));
    node->next = NULL;
    strcpy(node->name, name);
    node->is_CSEC = is_CSEC;
    node->address = address;

    return node;
}


/*
 * 목적 : src 문자열의 start 인덱스부터 size만큼 잘라내 dest 문자열에 저장한다.
 * 리턴값 : 없음
 */
void str_slice(char *dest, const char *src, int start, int size) {
    int j = 0;
    for (int i = start; i < start+size; ++i, ++j) {
        dest[j] = src[i];
    }
    dest[j] = '\0';
}

/*
 * 목적 : .obj 파일을 처음으로 읽으면서 파싱하여 H 레코드와 D레코드를 통해 ESTAB을 완성한다.
 * 리턴값 : 성공 시 -  1
 *        실패 시 -  -1
 */
int load_pass1(bucket* estab, char* file1, char* file2, char* file3, int files_num) {

    /*
     * 인자로 들어온 파일의 이름을 지역변수 배열에 저장한다.
     */
    char files[3][30];
    strcpy(files[0], file1);
    // 파일의 개수가 2개 이상이면 파일2를 저장
    if(files_num >= 2) {
        strcpy(files[1], file2);
        // 3개면 파일3도 저장
        if(files_num == 3) {
            strcpy(files[2], file3);
        }
    }

    // 파일의 개수만큼 반복한다.
    FILE* fp;
    for (int i = 0; i < files_num; ++i) {
        fp = fopen(files[i], "r");
        if(fp == NULL) {
            printf("cannot open file %s in load pass 1.\n", files[i]);
            return FAIL;
        }
        // 첫 파일이라면 CSADDR을 PROG_ADDRESS로 저장한다.
        if(i == 0) {
            CSADDR = PROG_ADDRESS;
        }

        while(1) {
            char line[70];
            char temp[7];
            int cur_index = 1;

            // .obj 파일로부터 한 줄씩 읽어들인다.
            fgets(line, 70, fp);
            line[strlen(line)-1] = '\0';
            if(feof(fp)) break;

            // H 레코드의 경우
            if(line[0] == 'H') {
                // 프로그램(또는 CSEC)의 이름 : H 레코드의 col 1~6까지 슬라이싱
                str_slice(temp, line, cur_index, 6);
                cur_index += 12;
                rtrim(temp);
                if(search_ES(estab, temp) != FAIL) {
                    printf("duplicate external symbol error.\n");
                    return FAIL;
                }
                ES_node* node = create_ESnode(temp, YES, CSADDR);

                // 프로그램(또는 CSEC)의 길이 : col 13~18까지 슬라이싱
                str_slice(temp, line, cur_index, 6);
                CS_LENGTH = node->length = (int)strtol(temp, NULL, 16);

                // 완성된 node를 ESTAB에 삽입한다.
                insert_ES(estab, node);
            }
            // D 레코드의 경우
            else if (line[0] == 'D') {
                int addr = 0;
                for (; cur_index < strlen(line); cur_index += 6) {
                    char name[7];
                    // EXTDEF Symbol name을 나타내는 column의 경우
                    if(cur_index % 12 == 1) {
                        str_slice(temp, line, cur_index, 6);
                        rtrim(temp);
                        if(search_ES(estab, temp) != FAIL) {
                            printf("duplicate external symbol error.\n");
                            return FAIL;
                        }
                    }
                    // EXTDEF Symbol의 주소를 나타내는 column의 경우
                    else {
                        strcpy(name, temp);
                        str_slice(temp, line, cur_index, 6);

                        // 주소에 CSADDR을 더한 값을 저장한다.
                        addr = (int)strtol(temp, NULL, 16) + CSADDR;
                        ES_node* node = create_ESnode(name, NO, addr);
                        insert_ES(estab, node);

                        addr = 0;
                        temp[0] = '\0';
                    }
                }
            }
            // 이외의 레코드는 pass1에서 다루지 않으므로 break한다.
            else break;
        }
        // CSADDR에 CS_LENGTH를 더해 다음 CS의 시작 주소로 설정한다.
        CSADDR += CS_LENGTH;
    }
    return SUCCESS;
}

int load_pass2(bucket* estab, char* file1, char* file2, char* file3, int files_num) {
    /*
     * 인자로 들어온 파일의 이름을 지역변수 배열에 저장한다.
     */
    char files[3][30];
    strcpy(files[0], file1);
    // 파일의 개수가 2개 이상이면 파일2를 저장
    if(files_num >= 2) {
        strcpy(files[1], file2);
        // 3개면 파일3도 저장
        if(files_num == 3) {
            strcpy(files[2], file3);
        }
    }

    // 파일의 개수만큼 반복한다.
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

            // obj 파일로부터 한 줄씩 읽어들인다.
            fgets(line, 70, fp);
            line[strlen(line)-1] = '\0';
            if(feof(fp)) break;

            // D 레코드거나 . 이면 건너뛴다.
            if(line[0] == 'D' || line[0] == '.') continue;

            // H 레코드의 경우 해당 주소를 ref_num 배열의 인덱스 1에 저장한다.
            else if(line[0] == 'H') {
                str_slice(temp, line, 1, 6);
                rtrim(temp);
                if((CSADDR = search_ES(estab, temp)) == FAIL) {
                    printf("cannot search prog name in estab.\n");
                    return FAIL;
                }
                ref_num[i][1] = CSADDR;
            }
            // R 레코드의 경우
            else if(line[0] == 'R') {
                int index;
                while(cur_index < strlen(line)) {
                    // ref_num 을 가리키는 column
                    if(cur_index % 8 == 1) {
                        // 해당 column을 슬라이싱해 index에 저장한다.
                        str_slice(temp, line, cur_index, 2);
                        index = (int)strtol(temp, NULL, 16);
                        cur_index+=2;
                    }
                    // EXTREF Symbol을 나타내는 column
                    else if (cur_index % 8 == 3) {
                        str_slice(temp, line, cur_index, 6);
                        rtrim(temp);
                        // 해당 symbol의 이름을 estab에서 검색해 주소를 ref_num의 index에 저장한다.
                        if((ref_num[i][index] = (search_ES(estab, temp))) == FAIL) {
                            printf("undefined external symbol.\n");
                            return FAIL;
                        }
                        cur_index += 6;
                    }
                }
            }
            // T 레코드의 경우
            else if(line[0] == 'T') {
                int T_startaddr, T_size, current_addr;
                unsigned int byte_value;

                // T 레코드의 시작 주소를 슬라이싱해 저장한다.
                str_slice(temp, line, cur_index, 6);
                cur_index += 6;
                T_startaddr = (int)strtol(temp, NULL, 16) + CSADDR;
                current_addr = T_startaddr;

                // T 레코드의 바이트 크기를 슬라이싱해 저장한다.
                str_slice(temp, line, cur_index, 2);
                T_size = (int)strtol(temp, NULL, 16);
                cur_index += 2;

                // T 레코드에서 한 바이트씩 값을 슬라이싱해서 메모리에 해당 주소에 저장한다.
                for (int j = 0; j < T_size; ++j) {
                    str_slice(temp, line, cur_index, 2);
                    byte_value = (unsigned int)strtol(temp, NULL, 16);
                    memset(&memory[current_addr], byte_value, 1);
                    current_addr++;
                    cur_index+=2;
                }
            }
            // M 레코드의 경우 modify를 수행한다.
            else if(line[0] == 'M') {
                modify(temp, line, cur_index, ref_num, i);
            }
            else break;
        }
    }
    return SUCCESS;
}

/*
 * 목적 : M 레코드에 맞게 modify를 수행한다.
 * 리턴값 : 없음
 */
void modify(char* temp, char* line, int cur_index, int ref_num[][6], int i) {
    int mod_target_addr, half_bytes_num, index;
    int origin_value = 0, mod_value = 0;

    // modify해야하는 주소를 슬라이싱해 저장한다.
    str_slice(temp, line, cur_index, 6);
    mod_target_addr = (int)strtol(temp, NULL, 16) + CSADDR;
    cur_index += 6;

    // modify가 필요한 half byte의 크기를 저장한다.
    str_slice(temp, line, cur_index, 2);
    half_bytes_num = (int)strtol(temp, NULL, 16);
    cur_index+= 2;

    // modify에 필요한 ref_num의 index를 슬라이싱해 저장한다.
    str_slice(temp, line, cur_index+1, 2);
    index = (int)strtol(temp, NULL, 16);

    // 덧셈이 필요한 경우
    if(line[cur_index] == '+') {
        mod_value += ref_num[i][index];
    }
    // 뺄셈이 필요한 경우
    else {
        mod_value -= ref_num[i][index];
    }

    // half bytes의 크기가 홀수인 경우
    if(half_bytes_num % 2 == 1) {
        int temp_value = memory[mod_target_addr];
        int or_value = 0;

        // mod_target_addr부터 수정해야하는 범위까지의 메모리를 읽어들여온다.
        for (int j = 0; j <= half_bytes_num / 2; ++j) {
            origin_value = origin_value << 8;
            origin_value = (origin_value | memory[mod_target_addr+j]);
        }

        // mod_target_addr의 middle half byte부터 수정해야하므로 왼쪽 half byte를 따로 저장해놓는다.
        temp_value = temp_value & 0xF0;
        // 따로 저장해놓은 값을 왼쪽으로 shift하여 빈 공간을 만들어 둔다.
        temp_value = temp_value << (8*(half_bytes_num/2));

        // half byte num 크기만큼 복사하기 위해 모든 비트가 1인 or_value를 만든다.
        for (int j = 0; j < half_bytes_num; ++j) {
            or_value = or_value << 4;
            or_value = or_value | 0xF;
        }

        // or_value와 & 연산을 통해 half byte num만큼의 값을 만든다.
        origin_value = origin_value & or_value;
        // 수정해야하는 값과 더해 값을 수정한다.
        origin_value += mod_value;
        // 미리 빼놓은 값과 | 연산을 통해 합쳐준다.
        origin_value = temp_value | origin_value;
    }
    // half bytes의 크기가 짝수인 경우
    else {
        // mod_target_addr부터 수정해야하는 범위까지의 메모리를 읽어들여온다.
        for (int j = 0; j < half_bytes_num / 2; ++j) {
            origin_value = origin_value << 8;
            origin_value = (origin_value | memory[mod_target_addr + j]);
        }
        // 수정해야하는 값과 더해 값을 수정한다.
        origin_value += mod_value;
   }

    // 수정해야하는 값을 임시 배열에 저장해놓는다.
    unsigned char* src = (unsigned char*)malloc(sizeof(unsigned char)*(half_bytes_num+1)/2);
    // 배열의 맨 뒤부터 LSB의 한 바이트씩 저장한다.
    for (int j = ((half_bytes_num + 1) / 2) - 1; j >= 0; --j) {
        src[j] = cut_by_byte_from_last(origin_value);
        origin_value = origin_value >> 8;
    }

    // 수정된 값을 메모리에 올린다.
    memcpy(&memory[mod_target_addr], src, (half_bytes_num+1)/2);
}

/*
 * 목적 : 입력받은 값의 LSB에서부터 한 바이트씩 자른다.
 * 리턴값 : 자른 한 바이트를 반환한다.
 */
unsigned int cut_by_byte_from_last(unsigned int number) {
    unsigned int temp = number & 0xFF;
    return temp;
}

/*
 * 목적 : 메모리에 올라온 프로그램을 실행한다.
 * 리턴값 : 성공 시 - 1
 *        실패 시 - -1
 *        BP에 의해 중단된 경우 - PC
 */
int run(int* BP_list, int BP_count, bucket* optab) {
    while (PC != PROG_ADDRESS + PROG_LENGTH) { // 수정
        int opcode, ni_bits, xbpe;
        int current_byte;
        char temp[10];
        int x,b,p,e;
        int TA;
        int j_flag = NO;
        int sign_bit = 1;

        // BP에 의해 중단되지 않았고, BP에 해당하는 PC의 경우
        if(BP_flag == NO && check_BP(BP_list, BP_count) == YES) {
            // 레지스터를 출력한다.
            print_registers();
            // BP_flag를 set하여 무한으로 중단되지 않도록 한다.
            BP_flag = YES;
            return PC;
        }
        BP_flag = NO;

        // 메모리로부터 PC에 위치한 한 바이트를 읽어들인다.
        current_byte = memory[PC];

        // ni bit를 제외하여 opcode를 얻어낸다.
        opcode = current_byte >> 2;
        opcode <<= 2;

        ni_bits = current_byte & 0x03;

        // opcode를 검색하여 해당하는 mnemonic을 찾아낸다.
        search_by_opcode(optab, opcode, temp);
        if(strcmp(temp, "FAIL") == 0) {
            printf("cannot disassemble opcode.\n");
            return FAIL;
        }
        // 해당 mnemonic으로 검색하여 해당하는 node를 찾아낸다.
        hash_node* opnode = search_opcode(optab, temp);

        // instruction format이 2 형식일 경우
        if(opnode->format[0] == 2) {
            // 다음 Loc을 PC+2로 설정하고 TA를 다음 한 바이트로 저장한다.
            nextPC = PC+2;
            current_byte = memory[PC+1];
            TA = current_byte;
        }
        // 3 또는 4 형식일 경우
        else {
            current_byte = memory[PC+1];
            // 왼쪽 half byte로부터 xbpe 비트 정보를 얻어낸다.
            xbpe = current_byte >> 4;
            x = xbpe >> 3;
            b = (xbpe >> 2) & 0x01;
            p = (xbpe >> 1) & 0x01;
            e = xbpe & 0x01;

            // 오른쪽 half 바이트를 따로 저장해놓는다.
            TA = (current_byte & 0x0F) << 8;

            // extended format (4 형식)일 경우
            if(e == 1) {
                // 다음 loc를 PC+4로 저장한다.
                nextPC = PC+4;
                // 뒤의 2바이트를 더 읽어들여 TA와 합쳐준다.
                TA <<= 8;
                current_byte = memory[PC+2];
                TA = TA | (current_byte << 8);
                current_byte = memory[PC+3];
                TA = TA | current_byte;
            } else {
                // 다음 loc를 PC+3으로 저장한다.
                nextPC = PC+3;
                // 뒤의 한 바이트를 더 읽어 TA와 합쳐준다.
                current_byte = memory[PC+2];
                TA = TA | current_byte;
            }
            // sign bit이 1이면 sign bit을 extend하여 음수로 만들어준다.
            sign_bit = TA >> 11;
            if(sign_bit == 1) {
                TA |= (int)0xFFFFF000;
            }

            // x bit이 1이면 TA에 X 레지스터 값을 더한다.
            if (x == 1) {
                TA += X;
            }

            // relative addressing에 따라 해당하는 레지스터 값을 TA에 더한다.
            if(b == 1 && p == 0) { // base relative
                TA += B;
            } else if(b == 0 && p == 1) { // pc relative
                TA += nextPC;
            }
        }

        // opcode에 따른 instruction을 수행한다.
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
        // jump instruction이 아니라면 PC를 nextPC로 저장한다.
        if(j_flag == NO) {
            PC = nextPC;
        }
    }

    return SUCCESS;
}

/*
 * 목적 : OPTAB에서 opcode로 검색하여 해당 mnemonic을 temp에 저장한다.
 * 리턴값 : 없음
 */
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

/*
 * 목적 : store instruction의 경우 store을 수행한다. reg에 있는 값을 메모리에 저장한다.
 * 리턴값 : 없음
 */
void store(const int* reg, int TA, int ni_bits) {
    // 레지스터의 각 바이트 값을 따로 저장한다.
    int first_byte = *reg >> (8*2);
    int second_byte = (*reg >> 8) & 0xFF;
    int third_byte = *reg & 0xFF;

    // ni_bits에 맞게 각 바이트 값을 메모리에 저장한다.
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

/*
 * 목적 : load instruction의 경우 load를 수행한다. 메모리의 값을 reg에 저장한다.
 * 리턴값 : 없음
 */
void load(int* reg, int TA, int ni_bits) {
    // ni_bits에 맞게 메모리의 각 바이트 값을 레지스터에 저장한다.
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

/*
 * 목적 : BP_list를 순회하며 현재 PC가 BP 중에 있는지 검사한다.
 * 리턴값 : 있으면 1을 없으면 0을 반환한다.
 */
int check_BP(const int* BP_list, int BP_count) {
    for (int i = 0; i < BP_count; ++i) {
        if(PC == BP_list[i]+PROG_ADDRESS) {
            return YES;
        }
    }
    return NO;
}

/*
 * 목적 : 각 레지스터의 현재 값을 출력한다.
 * 리턴값 : 없음
 */
void print_registers() {
    printf("A : %06X\tX : %06X\n", A, X);
    printf("L : %06X\t\bPC : %06X\n", L, PC);
    printf("B : %06X\tS : %06X\n", B, S);
    printf("T : %06X\n", T);
}

/*
 * 목적 : jump instruction의 경우 jump를 수행해 PC를 지정된 위치로 이동시킨다.
 * 리턴값 : 없음
 */
void jump(int TA, int ni_bits) {
    // ni_bits에 맞게 PC를 이동시킨다.
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