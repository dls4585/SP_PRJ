#include "20150195.h"

/*
 * 목적 : 리스트 초기화
 * 리턴값 : 없음
 */
void list_init(List* list) {
    list->head = NULL;
    list->tail = NULL;
}

/*
 * 목적 : 리스트에 필요한 노드를 생성한다.
 * 리턴값 : 생성한 노드
 */
Node* create_Node(char cmd_history[4][MAX_CMD_LEN], int i) {
    Node* node = (Node*)malloc(sizeof(Node));
    // 파라미터로 받은 명령어를 노드에 적절히 정제해 붙인다.
    for (int j = 0; j < i; ++j) {
        if(j == 0) {
            sprintf(node->cmd_history, "%s", cmd_history[j]);
        }
        else if(j == 1) {
            sprintf(node->cmd_history, "%s %s", node->cmd_history, cmd_history[j]);
        }
        else {
            sprintf(node->cmd_history, "%s, %s", node->cmd_history, cmd_history[j]);
        }
    }
    node->next = NULL;
    return node;
}

/*
 * 목적 : 리스트의 마지막 노드 뒤에 push back한다.
 * 리턴값 : 없음
 */
void list_push_back(List* list, Node* node) {
    // 리스트가 비어있을 때,
    if(list->head == NULL && list->tail == NULL) {
        list->head = list->tail = node;
    }
    // 리스트에 노드가 있을 때 제일 마지막 노드 뒤에 추가한다.
    else {
        list->tail->next = node;
        list->tail = node;
    }
}

/*
 * 목적 : cmd의 종류에 맞는 tokens의 개수에 따라 적절한 명령어가 입력됐는지 아닌지 검사한다.
 * 리턴값 : valid - 1 / invalid - 0
 */
int cmd_valid_check(int tokens, int cmd_case) {
    switch (cmd_case) {
        case QUIT: // quit
        case HELP: // help
        case DIRECTORY: // dir
        case HISTORY: // history
        case RESET: // reset
        case OPLIST: // opcodelist
        case SYMBOL:
            if(tokens > 1) {
                printf("Wrong command format, use help for command information\n");
                return 0;
            }
            break;
        case DUMP: // dump
            if(tokens > 3) {
                printf("Wrong command format, use help for command information\n");
                return 0;
            }
            break;
        case EDIT: // edit
            if(tokens != 3) {
                printf("Wrong command format, use help for command information\n");
                return 0;
            }
            break;
        case FILL: // fill
            if(tokens != 4) {
                printf("Wrong command format, use help for command information\n");
                return 0;
            }
            break;
        case MNEMONIC: // opcode mnemonic
        case TYPE:
        case ASSEMBLE:
            if(tokens != 2) {
                printf("Wrong command format, use help for command information\n");
                return 0;
            }
            break;
        default:
            return 1;
    }
    return 1;
}

/*
 * 목적 : 명령어 뒤의 인자가 적합한 인자인지 아닌지 검사한다.
 * 리턴값 : valid - 1 / invalid - -1
 */
int args_check(char* args) {
    // 인자 내에 공백 또는 탭 문자가 없어야 한다.
    if(strstr(args, " ")!= NULL || strstr(args, "\t") != NULL) {
        printf("Wrong arguments, arguments must be in range of 0x0 to 0xFFFFF\n");
        return FAIL;
    }
    // 인자는 숫자 또는 대소문자의 영어여야 한다.
    for (int i = 0; i < strlen(args); ++i) {
        if((args[i] >= 48 && args[i] <= 57) || (args[i] >= 65 && args[i] <= 70) || (args[i] >= 97 && args[i] <= 102)) {
            continue;
        } else {
            printf("Wrong arguments, arguments must be in range of 0x0 to 0xFFFFF\n");
            return FAIL;
        }
    }
    return SUCCESS;
}

/*
 * 목적 : 다음 명령어를 받기 위해 입력 스트림을 비우고, cmd_token 배열과 cmd 배열을 초기화한다.
 * 리턴값 : 없음
 */
void clear(char* cmd, char cmd_token[][MAX_CMD_LEN], int i) {
    for (int j = 0; j < i; ++j) {
        strcpy(cmd_token[j], "\0");
    }
    strcpy(cmd, "\0");
    rewind(stdin);
}

/*
 * 목적 : 명령어의 왼, 오른쪽의 공백을 없앤다.
 * 리턴값 : 없음
 */
void trim_cmd(char* cmd) {
    rtrim(cmd);
    ltrim(cmd);
}
/*
 * 목적 : 왼쪽의 공백을 없앤다.
 * 리턴값 : 없음
 */
void ltrim(char* cmd) {
    int index = 0, cmd_len = (int)strlen(cmd);
    char trimmed[MAX_CMD_LEN];

    strcpy(trimmed, cmd);
    // 왼쪽부터 시작해 공백 또는 탭 문자가 나오지 않을 때까지 탐색한 후
    for (int i = 0; i < cmd_len; ++i) {
        if(trimmed[i] == ' ' || trimmed[i] == '\t') {
            index++;
        }
        else {
            break;
        }
    }
    // 공백이 아닌 인덱스부터의 문자열을 원래의 문자열에 복사한다.
    strcpy(cmd, trimmed+index);
}

/*
 * 목적 : 오른쪽의 공백을 없앤다.
 * 리턴값 : 없음
 */
void rtrim(char* cmd) {
    int cmd_len = (int)strlen(cmd);
    char trimmed[MAX_CMD_LEN];

    strcpy(trimmed, cmd);
    // 오른쪽부터 시작해 공백 또는 탭 문자나 개행문자가 아닐 떄까지 탐색한 후
    for (int i = cmd_len-1; i >= 0; --i) {
        if(trimmed[i] == ' ' || trimmed[i] == '\t' || trimmed[i] == '\n') {
            continue;
        }
        else {
            // 공백이 아닌 인덱스+1에 널 문자를 삽입함으로써 문자열의 종료 구간을 설정한다.
            trimmed[i+1] = '\0';
            break;
        }
    }
    strcpy(cmd, trimmed);
}

/*
 * 목적 : 명령어가 소문자로 이루어져있는지 검사한다.
 * 리턴값 : 검사 통과 - 1 / 소문자가 아닌 글자 발견 - -1
 */
int is_lower(char* cmd) {
    for (int i = 0; i < strlen(cmd); ++i) {
        if(!islower(cmd[i])){
            return FAIL;
        }
    }
    return SUCCESS;
}

/*
 * 목적 : 명령어가 대문자로 이루어져 있는지 검사한다.
 * 리턴값 : 검사 통과 - 1 / 대문자가 아닌 글자 발견 - -1
 */
int is_upper(char* cmd) {
    for (int i = 0; i < strlen(cmd); ++i) {
        if (!isupper(cmd[i])) {
            return FAIL;
        }
    }
    return SUCCESS;
}

/*
 * 목적 : 해쉬테이블 초기화
 * 리턴값 : 초기화 성공 - 1 / 실패 - -1
 */
int optab_init(bucket* hashtable) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        hashtable[i].count = 0;
        hashtable[i].head = NULL;
    }
    // opcode.txt 파일을 읽어들여 해쉬테이블을 구성한다.
    FILE* fp = fopen("opcode.txt", "r");
    if(fp == NULL) {
        printf("opcode.txt file does not exist\n");
        return FAIL;
    }
    while(1) {
        char line[50];
        char line_tokens[3][10];
        int tokens = 0;
        hash_node* hash = (hash_node*)malloc(sizeof(hash_node));
        hash->format[0] = 0;
        hash->format[1] = 0;
        hash->next = NULL;

        fgets(line, 50, fp); // 파일로부터 한 라인씩 읽어들인다.
        if(feof(fp)) break;

        int len = (int)strlen(line);
        line[len-1] = line[len-2] = '\0'; // 파일이 \r\n으로 끝나므로 해당 문자를 없앤다.

        // 탭 문자를 통해 구분되고 있으므로 탭 문자를 구분자로 tokenizing 한다.
        char* ptr = strtok(line, "\t");
        while(ptr != NULL) {
            trim_cmd(ptr); // 탭 문자 이외에도 공백이 있으므로 해당 공백을 없애준다.
            strcpy(line_tokens[tokens], ptr);
            ptr = strtok(NULL, "\t");
            tokens++;
        }
        if(tokens != 3) {
            printf("file read is going wrong\n");
            return FAIL;
        }

        // 파일 내의 opcode 와 mnemonic을 해쉬 노드의 멤버 변수로 설정한다.
        hash->opcode = (int)strtol(line_tokens[0], NULL, 16);
        strcpy(hash->mnemonic, line_tokens[1]);

        // opcode 의 포맷을 설정한다.
        if(strlen(line_tokens[2]) < 2) {
            hash->format[0] = (int)strtol(line_tokens[2], NULL, 10);
        }
        // 3/4 처럼 두개의 포맷을 가질 수 있는 경우 둘 모두 저장한다.
        else {
            ptr = strtok(line_tokens[2], "/");
            for (int i = 0; i < 2; ++i) {
                hash->format[i] = (int)strtol(ptr, NULL, 10);
                ptr = strtok(NULL, "/");
            }
        }

        insert_opcode(hashtable, hash);
    }
    fclose(fp);
    return SUCCESS;
}

/*
 * 목적 : 해쉬 노드를 해쉬 테이블 내에 삽입한다.
 * 리턴값 : 없음
 */
void insert_opcode(bucket* hashtable, hash_node* hash) {
    // 해쉬 함수를 통해 인덱스를 얻는다.
    int hash_index = hash_function(hash->mnemonic);
    // 새로 삽입되는 해쉬 노드가 해당 인덱스의 리스트의 head가 된다.
    if(hashtable[hash_index].count == 0) {
        hashtable[hash_index].head = hash;
        hashtable[hash_index].count++;
    }
    else {
        hash->next = hashtable[hash_index].head;
        hashtable[hash_index].head = hash;
        hashtable[hash_index].count++;
    }
}

// 해쉬 함수
// field 각 자리의 ascii값 * 자리수 번호를 모두 한 후
// 해쉬 사이즈로 나눈 나머지를 반환음
/*
 * 목적 : 해쉬 테이블의 key 값을 구한다.
 * 리턴값 : 해쉬 테이블의 key(index)
 */
int hash_function(char* field) {
    int len = (int)strlen(field), ret = 0;
    for (int i = 0; i < len; ++i) {
        ret += (field[i] * (i + 1));
    }
    ret %= HASH_SIZE;
    return ret;
}

/*
 * 목적 : 해쉬 테이블을 순회하며 파라미터의 mnemonic에 해당하는 해쉬 노드를 찾고 그 opcode를 반환한다.
 * 리턴값 : opcode / 실패시 - -1
 */
hash_node* opcode_search(bucket* hashtable, char* mnemonic) {
    int hash_index = hash_function(mnemonic);
    hash_node* current = hashtable[hash_index].head;
    for (; current != NULL; current = current->next) {
        if(strcmp(current->mnemonic, mnemonic) == 0)
            return current;
    }
    return NULL;
}

/*
 * 목적 : 명령어로 공백과 탭 문자가 같이 들어올 수 있는데, 모두 공백으로 바꿔준다.
 * 리턴값 : 없음
 */
void replaceTab(char* string) {
    char* find;
    while(1) {
        find = strchr(string, '\t');
        if(!find) return;
        strncpy(find, " ",1);
    }
}

/*
 * 목적 : 해쉬 테이블인 symbol table을 초기화한다.
 * 리턴값 : 없음
 */
void symtab_init(bucket* symtab) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        symtab[i].s_head = NULL;
        symtab[i].count = 0;
    }
}

/*
 * 목적 : symbol table에 symbol node를 삽입한다.
 * 입력값 : 없음
 */
void insert_sym(bucket* symtab, symbol_node* symbol) {
    int index = hash_function(symbol->name) ;
    if(symtab[index].count == 0) {
        symtab[index].s_head = symbol;
        symtab[index].count++;
    }
    else {
        symbol->next = symtab[index].s_head;
        symtab[index].s_head = symbol;
        symtab[index].count++;
    }
}

/*
 * 목적 : symbol table을 순회하며 인자의 symbol name과 일치하는 노드를 찾아 해당 주소값을 반환한다.
 * 리턴값 : symbol node의 주소값
 */
int symbol_search(bucket* symtab, char* symbol_name) {
    int hash_index = hash_function(symbol_name);
    symbol_node* current = symtab[hash_index].s_head;
    for (; current != NULL; current = current->next) {
        if(strcmp(current->name, symbol_name) == 0)
            return current->LOCCTR;
    }
    return FAIL;
}

/*
 * 목적 : .asm 파일을 처음으로 읽으며 파싱하여 symbol table을 구성하고,
 *       .asm 파일의 각 라인의 정보를 담은 line list(doubly linked list)를 구성한다.
 * 리턴값 : 성공(1) / 실패(-1)
 */
int pass1(char* filename, bucket* optab, bucket* symtab, line_list* linelist, int* lines, int* prgm_len, int* error_flag){
    FILE* fp = fopen(filename, "r");
    if(fp == NULL) {
        printf("cannot open file %s\n", filename);
        return FAIL;
    }
    
    int start_addr;
    int LOCCTR = 0;
    int nextLOCCTR = 0;

    while(1) {
        char line[100];
        char line_token[3][30];
        int asm_tokens = 0;
        char* asm_ptr;
        char symbol[30]= "\0";
        char mnemonic[30]= "\0";
        char operand[30]= "\0";
        char operand2[30] = "\0";
        line_node* node;

        // .asm 파일로부터 한 줄씩 읽어들인다.
        fgets(line, 100, fp);
        if(feof(fp)) break;
        *lines+=5;

        int len = (int)strlen(line);
        line[len-1] = '\0';

        /*
         * 주석인 라인의 처리
         * 해당 라인이 주석일 경우 obj_code와 LOCCTR, operand는 존재하지 않고,
         * mnemonic 대신 주석의 내용을 넣어준다.
         */
        if(line[0] == '.') {
            memmove(&line[0], &line[1], strlen(line)-1);
            line[strlen(line)-1] = '\0';
            ltrim(line);
            node = create_line_node(*lines, NONE, ".", line, "\0", "\0");
            node->obj_code = NONE;
        }
        else {
            /*
             * 주석이 아닌 경우의 처리
             */

            // 읽은 문자열을 토큰화한다.
            asm_ptr = strtok(line, " ");
            while(asm_ptr != NULL) {
                strcpy(line_token[asm_tokens], asm_ptr);
                asm_tokens++;
                asm_ptr = strtok(NULL, ", ");
            }

            // opcode가 BASE인 경우
            // assemble 명령어가 아닌 BASE 레지스터의 값을 알려주는 용도이므로 LOCCTR이 없도록 처리한다.
            if(strcmp(line_token[0], "BASE") == 0) {
                node = create_line_node(*lines, NONE, "\0", "BASE", line_token[1], "\0");
                line_list_push_back(linelist, node);
                continue;
            }

            // token이 3개인 경우는
            // symbol mnemonic operand  또는
            // mnemonic operand1, operand2 인 경우이다.
            if(asm_tokens == 3) {
                // 첫 토큰이 mnemonic이 아니라면 symbol을 갖고 있는 라인으로 처리한다.
                if(opcode_search(optab, line_token[0]) == NULL) {
                    strcpy(mnemonic, line_token[1]);
                    strcpy(operand, line_token[2]);
                    strcpy(symbol, line_token[0]);

                    // 새로운 symbol node를 생성하고 초기화한다.
                    if(strcmp(mnemonic, "START") != 0 || strcmp(mnemonic, "END") != 0) {
                        symbol_node* symbolNode = (symbol_node*)malloc(sizeof(symbol_node));
                        strcpy(symbolNode->name, symbol);
                        symbolNode->LOCCTR = LOCCTR;

                        // 이미 있는 symbol이라면 중복된 symbol 정의이므로 에러 처리한다.
                        if(symbol_search(symtab, symbolNode->name) != -1) {
                            free(symbolNode);
                            *error_flag = 2;
                            break;
                        }
                        // 새로운 symbol을 symbol table에 삽입한다.
                        insert_sym(symtab, symbolNode);
                    }
                }
                else {
                    // symbol이 없고, operand가 2개이므로 이에 맞게 값을 복사한다.
                    strcpy(mnemonic, line_token[0]);
                    strcpy(operand, line_token[1]);
                    strcpy(operand2, line_token[2]);
                }
            }
            // token이 2개인 경우는 mnemonic operand의 형식이므로것
            // symbol이 없고 operand가 하나인 으로 처리한다.
            else if(asm_tokens == 2) {
                strcpy(mnemonic, line_token[0]);
                strcpy(operand, line_token[1]);
            }
            // token이 1개인 경우는
            // mnemonic만 있는 경우 이므로 이에 맞게 처리한다.
            else {
                strcpy(mnemonic, line_token[0]);
            }

            /*
             * LOCCTR의 계산
             */
            if(strcmp(mnemonic, "END") == 0); // END의 경우 아무런 처리를 하지 않는다.
            // START의 경우, 해당 프로그램의 시작주소를 알리는 것이므로 다음 라인의 주소값 또한 동일하게 설정한다.
            else if(strcmp(mnemonic, "START") == 0) {
                LOCCTR = (int)strtol(operand, NULL, 16);
                start_addr = LOCCTR;
                nextLOCCTR = LOCCTR;
            }
            // extended format 을 사용하는 경우
            // instruction format 4를 사용하고, 이는 4바이트이므로 다음 주소값은 4바이트를 증가시킨다.
            else if(mnemonic[0] == '+') {
                nextLOCCTR += 4;
            }
            // 이 외의 경우 mnemonic에 해당하는 format에 따라 계산한다.
            else {
                hash_node* opcode_node;
                // optab에 존재하는 mnemonic의 경우
                // 해당 format에 맞는 바이트만큼 증가시킨다.
                if((opcode_node = opcode_search(optab, mnemonic)) != NULL) {
                    if(opcode_node->format[0] == 1) {
                        nextLOCCTR += 1;
                    }
                    else if(opcode_node->format[0] == 2) {
                        nextLOCCTR += 2;
                    }
                    else {
                        nextLOCCTR += 3;
                    }
                }

                /*
                 * assember directive의 경우 해당 operand에 맞는 바이트 수를 증가시킨다.
                 */
                else if (strcmp(mnemonic, "RESW") == 0) {
                    int operands_len = (int)strtol(operand, NULL, 10);
                    nextLOCCTR += (3*operands_len); // operand로 들어온 값 * 3(word의 크기) 만큼 증가시킨다
                }
                else if (strcmp(mnemonic, "RESB") == 0) {
                    int operands_len = (int)strtol(operand, NULL, 10);
                    nextLOCCTR += operands_len; // operand로 들어온 값만큼 증가시킨다.
                }
                else if (strcmp(mnemonic, "BYTE") == 0) {
                    if(operand[0] == 'C') {
                        int operands_len = (int)strlen(operand);
                        nextLOCCTR += (operands_len-3); // character의 경우 한 자리가 한 바이트이므로 그 길이만큼 증가시킨다.
                    }
                    else if (operand[0] == 'X') {
                        int oplen = (int)strlen(operand)-3;
                        nextLOCCTR += (int)ceil(oplen/(double)2.0); // hex의 경우 두 자리가 한 바이트이므로 그만큼 증가시킨다.
                    }
                }
                else if(strcmp(mnemonic, "WORD") == 0) { // word의 경우 operand의 크기에 맞는 값을 증가시킨다.
                    if((int)strtol(operand, NULL,10) > 0xFFFFFF) {
                        *error_flag = 1;
                        break;
                    }
                    nextLOCCTR += 3;

                }
                // 해당하는 case가 없는 경우 에러 처리한다.
                else {
                    *error_flag = 1;
                    break;
                }
            }
            // 계산한 LOCCTR 등의 정보를 바탕으로 line node를 생성한다.
            node = create_line_node(*lines, LOCCTR, symbol, mnemonic, operand, operand2);
        }
        // line list에 추가한다.
        line_list_push_back(linelist, node);

        // LOCCTR를 다음 줄로 이동시킨다.
        LOCCTR = nextLOCCTR;
    }

    // linelist->tail == "END"
    // 그 이전의 노드의 LOCCTR에서 시작 주소를 빼서 프로그램의 길이를 계산한다.
    *prgm_len = linelist->tail->prev->LOCCTR - start_addr;
    fclose(fp);
    return SUCCESS;
}

/*
 * 목적 : pass1에서 구성한 linelist를 순회하며 object code를 계산하고, .lst / .obj 파일을 작성한다.
 * 리턴값 : 성공(1) / 실패(-1)
 */
int pass2(char* filename, bucket* optab, bucket* symtab, line_list* linelist, int* prgm_len) {
    char obj_file[30];
    char lst_file[30];
    filename[strlen(filename)-4] = '\0';
    sprintf(obj_file, "%s.obj", filename);
    sprintf(lst_file, "%s.lst", filename);


    int base = 0;
    int pc = 0;
    line_node* current = linelist->head;
    while(current != NULL) {
        int obj_code = 0, format = 0;
        int target_addr = 0;
        int b=0,p=0,x=0,e=0,n=0,i=0;

        char mnemonic[100];
        char symbol[30];
        char operand[30];
        char operand2[30];

        int is_mnemonic_byte = 0;

        strcpy(mnemonic, current->mnemonic);
        strcpy(symbol, current->symbol);
        strcpy(operand, current->operand[0]);
        strcpy(operand2, current->operand[1]);

        // line list의 마지막이므로 obj code와 locctr을 없는 것으로 설정하고 반복문을 탈출한다.
        if(strcmp(mnemonic, "END") == 0) {
            current->obj_code = NONE;
            current->LOCCTR = NONE;
            break;
        }
        // PC는 해당 라인의 다음 줄로 설정된다.
        pc = current->next->LOCCTR;
        if(strcmp(mnemonic, "START") != 0 && pc == NONE) {
            line_node* node = current->next;
            while(node->LOCCTR == NONE) {
                node = node->next;
            }
            pc = node->LOCCTR;
        }

        if(operand[strlen(operand)-1] == ',') {
            operand[strlen(operand)-1] = '\0';
        }

        /*
         * nixe 비트 설정
         */
        // set x bit
        if(strcmp(operand2, "X") == 0) {
            x = 1;
        }

        // set e bit
        if(current->mnemonic[0] == '+') {
            format = 4;
            e = 1;
            memmove(&mnemonic[0], &mnemonic[1], strlen(mnemonic) - 1);
            mnemonic[strlen(mnemonic) - 1] = '\0'; // + 제거
        }

        // set n, i bit
        if(operand[0] == '#') {
            i = 1; n = 0; // immediate
            memmove(&operand[0], &operand[1], strlen(operand)-1);
            operand[strlen(operand)-1] = '\0'; // # 제거
        }
        else if (operand[0] == '@') {
            n = 1; i = 0; // indirect
            memmove(&operand[0], &operand[1], strlen(operand)-1);
            operand[strlen(operand)-1] = '\0'; // @ 제거
        }
        else {
            i = 1; n = 1; // simple
        }

        /*
         * BASE 레지스터 관련 처리
         */
        // LDB 의 경우 base 레지스터를 설정해준다.
        if(strcmp(mnemonic, "LDB") == 0) {
            base = symbol_search(symtab, operand);
        }
        // base의 경우 assembly directive 이므로 넘어간다.
        else if(strcmp(mnemonic, "BASE") == 0){
            current->obj_code = NONE;
            current = current->next;
            continue;
        }

        hash_node* opcode_node;

        /*
         * Target Address 계산
         */
        // start의 경우 assembly directive 이므로 넘어간다.
        if(strcmp(mnemonic, "START") == 0) {
            current->obj_code = NONE;
            current = current->next;
            continue;
        }
        // optab에 존재하지 않는 mnemonic 인 경우
        else if((opcode_node = opcode_search(optab, mnemonic)) == NULL) {
            if(strcmp(mnemonic, "BYTE") == 0) {
                is_mnemonic_byte = 1;
                if(operand[0] == 'C') { // character의 경우
                    for(int j=2; j<strlen(operand)-1; j++) {
                        target_addr += (int)operand[j];
                        target_addr = target_addr << 8; // 한 글자(한 바이트)씩 왼쪽으로 shift한다.
                    }
                    target_addr = target_addr >> 8;
                }
                else if(operand[0] == 'X') { // hex의 경우
                    memmove(&operand[0], &operand[2], strlen(operand)-2);
                    operand[strlen(operand)-3] = '\0';
                    for (int j = 0; j < strlen(operand); ++j) {
                        if(operand[j] > 'F' || operand[j] < '0') { // 인자의 '0~F' 범위 처리
                            return FAIL;
                        }
                    }
                    target_addr = (int)strtol(operand, NULL, 16); // 해당 operand의 크기만큼 target address로 계산한다.
                }
            }
            else if(strcmp(mnemonic, "WORD") == 0) {
                is_mnemonic_byte = 1;
                target_addr = (int)strtol(operand, NULL, 10);
            }
            else {
                current->obj_code = NONE;
                current = current->next;
                continue;
            }
        }
        else { // optab에 존재하는 mnemonic인 경우
            obj_code = opcode_node->opcode;
            if((target_addr = symbol_search(symtab, operand)) == FAIL) { // operand가 symbol이 아니면
                target_addr = (int)strtol(operand, NULL, 10);
                // 레지스터인 경우 레지스터에 맞는 값을 target addr로 설정한다.
                if(strcmp(operand, "A") == 0) {
                    strcpy(operand,"0");
                }
                else if(strcmp(operand, "X") == 0) {
                    strcpy(operand,"1");
                }
                else if(strcmp(operand, "L") == 0) {
                    strcpy(operand,"2");
                }
                else if(strcmp(operand, "PC") == 0) {
                    strcpy(operand,"8");
                }
                else if(strcmp(operand, "SW") == 0) {
                    strcpy(operand,"9");
                }
                else if(strcmp(operand, "B") == 0) {
                    strcpy(operand,"3");
                }
                else if(strcmp(operand, "S") == 0) {
                    strcpy(operand,"4");
                }
                else if(strcmp(operand, "T") == 0) {
                    strcpy(operand,"5");
                }
                else if(strcmp(operand, "F") == 0) {
                    strcpy(operand,"6");
                }

                if(strlen(operand2) == 0) {
                    strcpy(operand2, "0");
                }
                // operand가 2개인 경우도 위와 같이 설정한다.
                else {
                    if(strcmp(operand2, "A") == 0) {
                        strcpy(operand2, "0");
                    }
                    else if(strcmp(operand2, "X") == 0) {
                        strcpy(operand2, "1");
                    }
                    else if(strcmp(operand2, "L") == 0) {
                        strcpy(operand2, "2");
                    }
                    else if(strcmp(operand2, "PC") == 0) {
                        strcpy(operand2, "8");
                    }
                    else if(strcmp(operand2, "SW") == 0) {
                        strcpy(operand2, "9");
                    }
                    else if(strcmp(operand2, "B") == 0) {
                        strcpy(operand2, "3");
                    }
                    else if(strcmp(operand2, "S") == 0) {
                        strcpy(operand2, "4");
                    }
                    else if(strcmp(operand2, "T") == 0) {
                        strcpy(operand2, "5");
                    }
                    else if(strcmp(operand2, "F") == 0) {
                        strcpy(operand2, "6");
                    }
                }

            }
            else {
                /*
                 * pc / base bit set
                 */
                int TAP = target_addr - pc;
                int TAB = target_addr - base;
                if(TAP <= 2047 && TAP >= -2048) { // pc relative
                    p = 1; b = 0;
                    target_addr -= pc;
                }
                else if(TAB >= 0 && TAB <= 4095) { // base relative
                    b = 1; p = 0;
                    target_addr -= base;
                }
                else { // extended, need to be modified
                    current->mod_bit = 1;
                    b = 0; p = 0;
                    e = 1;
                }
            }
        }

        if(is_mnemonic_byte == 1) {
            obj_code = target_addr;
        }
        else {
            // shift 연산을 통해 xbpe로 구성된 한 바이트를 만든다.
            int xbpe = (x << 3) | (b << 2) | (p << 1) | e;

            /*
             * format에 맞게 xbpe, obj_code, target_addr를 시프트하여 object code를 계산한다.
             */
            if(format == 4 || e == 1) {
                obj_code = obj_code | (n << 1) | i;
                obj_code = obj_code << 24;
                xbpe = xbpe << 20;
                obj_code += xbpe;
                obj_code = obj_code | target_addr;
            }
            else if(opcode_node->format[0] == 2) {
                obj_code = obj_code << 8;
                int r1 = (int)strtol(operand, NULL, 16);
                int r2 = (int)strtol(operand2, NULL, 16);


                obj_code = obj_code | (r1 << 4) | r2;
            } else if(opcode_node->format[0] == 3) {
                obj_code = obj_code | (n << 1) | i;
                obj_code = obj_code << 16;
                xbpe = xbpe << 12;
                obj_code += xbpe;
                if(target_addr < 0) {
                    target_addr = 0x1000 + target_addr;
                }
                obj_code = obj_code + target_addr;
            }
        }

        current->obj_code = obj_code;
        current = current->next;
    }

    /*
     * .lst, .obj 파일 작성
     */
    if(write_lst_file(linelist, lst_file) == FAIL) {
        return FAIL;
    }
    if(write_obj_file(linelist, obj_file, prgm_len) == FAIL) {
        return FAIL;
    }

    return SUCCESS;
}


/*
 * 목적 : .asm 파일의 각 라인 정보를 담은 노드의 doubly linked list인 line_list를 초기화한다.
 * 리턴값 : 없음
 */
void line_list_init(line_list* list) {
    list->head = NULL;
    list->tail = NULL;
}

/*
 * 목적 : line_list의 맨 마지막에 노드를 삽입한다.
 * 리턴값 : 없음
 */
void line_list_push_back(line_list* list, line_node* node) {
    if(list->head == NULL && list->tail == NULL) {
        list->head = list->tail = node;
    }
        // 리스트에 노드가 있을 때 제일 마지막 노드 뒤에 추가한다.
    else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
}

/*
 * 목적 : 인자로 들어온 정보를 바탕으로 line_node를 생성한다.
 * 리턴값 : 생성된 line_node의 pointer
 */
line_node* create_line_node(int line, int LOCCTR, char symbol[], char mnemonic[], char operand1[], char operand2[]) {
    line_node* node = (line_node*)malloc(sizeof(line_node));
    node->line = line;
    node->LOCCTR = LOCCTR;
    strcpy(node->symbol, symbol);;
    strcpy(node->mnemonic, mnemonic);
    strcpy(node->operand[0], operand1);
    strcpy(node->operand[1], operand2);
    node->obj_code = 0;
    node->mod_bit = 0;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

/*
 * 목적 : pass2에서 완성한 line_list를 바탕으로 .lst 파일을 작성한다.
 * 리턴값 : 성공(1) / 실패(-1)
 */
int write_lst_file(line_list* linelist, char* filename) {
    FILE* fp = fopen(filename, "w");
    if(fp == NULL) {
        printf("cannot open lst file to write\n");
        return FAIL;
    }
    line_node* current = linelist->head;

    while(current != NULL) {

        /*
         * line number를 출력
         */
        fprintf(fp, "%4d  ", current->line);

        /*
         * LOCCTR가 존재하면 출력
         * 아니면 공백 확보
         */
        if(current->LOCCTR != NONE) {
            fprintf(fp, "%04X    ", current->LOCCTR);
        }
        else {
            fprintf(fp, "%8s", "\0");
        }

        /*
         * symbol과 mnemonic 출력
         */
        fprintf(fp, "%-8s  ", current->symbol);
        fprintf(fp, "%-8s  ", current->mnemonic);

        /*
         * operand 출력
         */
        fprintf(fp, "%-8s", current->operand[0]);
        if(strlen(current->operand[1]) == 0) {
            fprintf(fp, "%8s", "\0");
        }
        else {
            int len = 8 - (int)strlen(current->operand[0]);
            fseek(fp, -1*len, SEEK_CUR);

            fprintf(fp, ", %-6s", current->operand[1]);
            for (int i = 0; i < 8-strlen(current->operand[0]); ++i) {
                fprintf(fp, " ");
            }
        }

        // object code 출력
        if(current->obj_code != NONE) {
            fprintf(fp, "%06X", current->obj_code);
        }

        fprintf(fp, "\n");
        current = current->next;
    }

    fclose(fp);
    return SUCCESS;
}

/*
 * 목적 : pass2에서 완성한 line_list를 바탕으로 .obj 파일을 작성한다.
 * 리턴값 : 성공(1) / 실패(-1)
 */
int write_obj_file(line_list* linelist, char* filename, const int* prgm_len) {
    FILE* fp = fopen(filename, "w");
    if(fp == NULL) {
        printf("cannot open obj file to write\n");
        return FAIL;
    }
    int* mod_byte_array = (int*)malloc(sizeof(int)*(linelist->tail->line / 5));
    int mod_byte_cnt = 0;
    int byte_cnt = 0;
    int line_start_addr = 0;
    int start_addr = 0;
    int new_line_flag = 1;
    int mod_line_flag = 0;
    char obj_code[70] = "\0";
    line_node* current = linelist->head;


    while(current != NULL) {
        int line_len = 0;
        /*
         * START의 경우 Header Record 출력
         */
        if(strcmp(current->mnemonic, "START") == 0) {
            start_addr = (int)strtol(current->operand[0], NULL, 16);
            fprintf(fp, "H%-6s%06X%06X\n", current->symbol, start_addr, *prgm_len);
        }
        /*
         * End 의 경우 만든 obj_code들을 출력
         * modification record가 시작해야 하므로 해당 flag를 set한다.
         */
        else if(strcmp(current->mnemonic, "END") == 0) {
            fprintf(fp, "%02X", byte_cnt);
            fprintf(fp, "%s\n", obj_code);
            strcpy(obj_code, "\0");
            mod_line_flag = 1;
            new_line_flag = 0;
        }
        /*
         * RESB, RESB를 만나면
         * 만들어놓은 obj_code 가 있으면 출력한다.
         * 개행을 위해 new_line_flag를 set한다.
         */
        else if(strcmp(current->mnemonic, "RESB") == 0 || strcmp(current->mnemonic, "RESW") == 0) {
            if(strlen(obj_code) > 0) {
                fprintf(fp, "%02X", byte_cnt);
                fprintf(fp, "%s\n", obj_code);
                strcpy(obj_code, "\0");
            }
            new_line_flag = 1;
            byte_cnt = 0;
        }
        else if(current->obj_code == NONE);
        /*
         * new_line_flag가 set되면
         * 새로운 Text Record를 시작한다.
         */
        else if(new_line_flag) {
            fprintf(fp, "T%06X", current->LOCCTR);
            line_start_addr = current->LOCCTR;
            new_line_flag = 0;
            continue;
        }
        /*
         * 이 외의 경우 obj_code를 만든다.
         */
        else {
            // modification bit가 1이면
            // modification record에 사용해야하므로 해당 배열에 주소를 저장한다.
            if(current->mod_bit == 1) {
                mod_byte_array[mod_byte_cnt++] = current->LOCCTR - start_addr;
            }
            /*
             * 해당 라인의 object code의 범위에 맞게 출력한다.
             */
            if(current->obj_code <= 0xFF) {
                sprintf(obj_code, "%s%02X", obj_code, current->obj_code);
            }
            else if(current->obj_code <= 0xFFFF) {
                sprintf(obj_code, "%s%04X", obj_code, current->obj_code);
            }
            else {
                sprintf(obj_code, "%s%06X", obj_code, current->obj_code);
            }

            /*
             * Text Record에 적히는 라인의 길이(바이트의 수) 계산
             */
            int next_bytes;
            line_node* node = current->next;
            if(node->next != NULL) {
                while(node->LOCCTR == NONE){
                    node = node->next;
                }
                byte_cnt += node->LOCCTR - current->LOCCTR;
                next_bytes = node->LOCCTR;
                do {
                    node = node->next;
                }while(node->LOCCTR == NONE && node->next != NULL);
            }
            // current node가 END 직전의 노드일 때
            else {
                if(current->obj_code <= 0xFF) {
                    byte_cnt+=1;
                }
                else if(current->obj_code <= 0xFFFF) {
                    byte_cnt+=2;
                }
                else if(current->obj_code <= 0xFFFFFF) {
                    byte_cnt+=3;
                }
                else {
                    byte_cnt+=4;
                }
            }

            // current node가 END 직전의 노드일 때 next_bytes 계산
            if(node == linelist->tail) {
                node = node->prev;
                if(node->obj_code <= 0xFF) {
                    next_bytes = 1;
                }
                else if(node->obj_code <= 0xFFFF) {
                    next_bytes = 2;
                }
                else if(node->obj_code <= 0xFFFFFF) {
                    next_bytes = 3;
                }
                else {
                    next_bytes = 4;
                }
            }
            else { // 이 외의 경우 다다음 라인의 LOCCTR에서 다음 라인의 LOCCTR을 빼면 다음 라인의 bytes 수가 된다.
                next_bytes = node->LOCCTR - next_bytes;
            }

            /*
             * 현재까지의 bytes와 다음 라인의 bytes를 더했을 때 0x1E를 넘어가면 현재까지만 출력한다
             * 개행을 위해 flag를 set한다.
             */
            if(byte_cnt + next_bytes > 0x1E) {
                fprintf(fp, "%02X", byte_cnt);
                fprintf(fp, "%s\n", obj_code);
                strcpy(obj_code, "\0");
                new_line_flag = 1;
                byte_cnt = 0;
            }
        }
        current = current->next;
    }

    /*
     * modification record
     * 현재는 5 half bytes인 경우만 존재하므로 05를 출력
     */
    if(mod_line_flag) {
        for (int i = 0; i < mod_byte_cnt; ++i) {
            fprintf(fp, "M%06X05\n", mod_byte_array[i]+1);
        }
    }

    // END record 출력
    fprintf(fp, "E%06X\n", start_addr);
    fclose(fp);
    return SUCCESS;
}