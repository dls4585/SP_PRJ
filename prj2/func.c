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
 * 리턴값 : valid - 1 / invalid - 0
 */
int args_check(char* args) {
    // 인자 내에 공백 또는 탭 문자가 없어야 한다.
    if(strstr(args, " ")!= NULL || strstr(args, "\t") != NULL) {
        printf("Wrong arguments, arguments must be in range of 0x0 to 0xFFFFF\n");
        return 0;
    }
    // 인자는 숫자 또는 대소문자의 영어여야 한다.
    for (int i = 0; i < strlen(args); ++i) {
        if((args[i] >= 48 && args[i] <= 57) || (args[i] >= 65 && args[i] <= 70) || (args[i] >= 97 && args[i] <= 102)) {
            continue;
        } else {
            printf("Wrong arguments, arguments must be in range of 0x0 to 0xFFFFF\n");
            return 0;
        }
    }
    return 1;
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
 * 리턴값 : 검사 통과 - 1 / 소문자가 아닌 글자 발견 - 0
 */
int is_lower(char* cmd) {
    for (int i = 0; i < strlen(cmd); ++i) {
        if(!islower(cmd[i])){
            return 0;
        }
    }
    return 1;
}

/*
 * 목적 : 명령어가 대문자로 이루어져 있는지 검사한다.
 * 리턴값 : 검사 통과 - 1 / 대문자가 아닌 글자 발견 - 0
 */
int is_upper(char* cmd) {
    for (int i = 0; i < strlen(cmd); ++i) {
        if (!isupper(cmd[i])) {
            return 0;
        }
    }
    return 1;
}

/*
 * 목적 : 해쉬테이블 초기화
 * 리턴값 : 초기화 성공 - 1 / 실패 - 0
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
        return 0;
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
            return 0;
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
    return 1;
}

/*
 * 목적 : 해쉬 노드를 해쉬 테이블 내에 삽입한다.
 * 리턴값 : 없
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

void symtab_init(bucket* symtab) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        symtab[i].s_head = NULL;
        symtab[i].count = 0;
    }
}

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

int symbol_search(bucket* symtab, char* symbol_name) {
    int hash_index = hash_function(symbol_name);
    symbol_node* current = symtab[hash_index].s_head;
    for (; current != NULL; current = current->next) {
        if(strcmp(current->name, symbol_name) == 0)
            return current->LOCCTR;
    }
    return FAIL;
}

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
        char opcode[30]= "\0";
        char operand[30]= "\0";
        char operand2[30] = "\0";
        line_node* node;

        fgets(line, 100, fp);
        if(feof(fp)) break;
        *lines+=5;

        int len = (int)strlen(line);
        line[len-1] = '\0';

        if(line[0] == '.') {
            memmove(&line[0], &line[1], strlen(line)-1);
            line[strlen(line)-1] = '\0';
            ltrim(line);
            node = create_line_node(*lines, NONE, ".", line, "\0", "\0");
            node->obj_code = NONE;
        }
        else {
            asm_ptr = strtok(line, " ");
            while(asm_ptr != NULL) {
                strcpy(line_token[asm_tokens], asm_ptr);
                asm_tokens++;
                asm_ptr = strtok(NULL, ", ");
            }

            if(strcmp(line_token[0], "BASE") == 0) {
                node = create_line_node(*lines, NONE, "\0", "BASE", line_token[1], "\0");
                line_list_push_back(linelist, node);
//                *line-=5;
                continue;
            }

            if(asm_tokens == 3) { // label 있음
                if(opcode_search(optab, line_token[0]) == NULL) { // symbol 이라는 말
                    strcpy(opcode, line_token[1]);
                    strcpy(operand, line_token[2]);
                    strcpy(symbol, line_token[0]);
                    symbol_node* symbolNode = (symbol_node*)malloc(sizeof(symbol_node));
                    strcpy(symbolNode->name, symbol);
                    symbolNode->LOCCTR = LOCCTR;
                    if(symbol_search(symtab, symbolNode->name) != -1) {
                        *error_flag = 2;
                        break;
                    }
                    insert_sym(symtab, symbolNode);
                }
                else { // operand 가 두개
                    strcpy(opcode, line_token[0]);
                    strcpy(operand, line_token[1]);
                    strcpy(operand2, line_token[2]);
                }
            }
            else if(asm_tokens == 2) {
                strcpy(symbol, "\0");
                strcpy(opcode, line_token[0]);
                strcpy(operand, line_token[1]);
            }
            else {
                strcpy(opcode, line_token[0]);
            }

            if(strcmp(opcode, "END") == 0);
            else if(strcmp(opcode, "START") == 0) {
                LOCCTR = (int)strtol(operand, NULL, 10);
                start_addr = LOCCTR;
                nextLOCCTR = LOCCTR;
            }
            else if(opcode[0] == '+') {
                nextLOCCTR += 4;
            }
            else {
                hash_node* opcode_node;
                if((opcode_node = opcode_search(optab, opcode)) != NULL) {
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
                else if (strcmp(opcode, "RESW") == 0) {
                    int operands_len = (int)strtol(operand, NULL, 10);
                    nextLOCCTR += (3*operands_len);
                }
                else if (strcmp(opcode, "RESB") == 0) {
                    int operands_len = (int)strtol(operand, NULL, 10);
                    nextLOCCTR += operands_len;
                }
                else if (strcmp(opcode, "BYTE") == 0) {
                    if(operand[0] == 'C') {
                        int operands_len = (int)strlen(operand);
                        nextLOCCTR += (operands_len-3);
                    }
                    else if (operand[0] == 'X') {
                        int oplen = (int)strlen(operand)-3;
                        nextLOCCTR += (int)ceil(oplen/(double)2.0);
                    }

                }
                else {
                    *error_flag = 1;
                    break;
                }
            }
            node = create_line_node(*lines, LOCCTR, symbol, opcode, operand, operand2);

        }

        line_list_push_back(linelist, node);

        LOCCTR = nextLOCCTR;
    }
    *prgm_len = linelist->tail->prev->LOCCTR - start_addr;
    fclose(fp);
    FILE* mid_file = fopen("mid_file.txt", "w");
    for (line_node* current = linelist->head; current != NULL ; current = current->next) {
        fprintf(mid_file, "%d\t%04X\t%s\t%s\t%s\t%s\n", current->line, current->LOCCTR, current->symbol, current->mnemonic, current->operand[0], current->operand[1]);
    }
    fclose(mid_file);
    return SUCCESS;
}

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

        char opcode[100];
        char symbol[30];
        char operand[30];
        char operand2[30];

        int is_opcode_byte = 0;

        strcpy(opcode, current->mnemonic);
        strcpy(symbol, current->symbol);
        strcpy(operand, current->operand[0]);
        strcpy(operand2, current->operand[1]);
        if(strcmp(opcode, "END") == 0) {
            current->obj_code = NONE;
            current->LOCCTR = NONE;
//            printf("%04d\t%04X\t%s\t%s\t%s\t%s\t%06X\n", current->line, current->LOCCTR, current->symbol,current->mnemonic, current->operand[0],
//                   current->operand[1], obj_code);
            break;
        }
        pc = current->next->LOCCTR;
        if(strcmp(opcode, "START") != 0 && pc == NONE) {
            pc = current->next->next->LOCCTR;
        }
        if(operand[strlen(operand)-1] == ',') {
            operand[strlen(operand)-1] = '\0';
        }

        // set x bit
        if(strcmp(operand2, "X") == 0) {
            x = 1;
        }

        // set e bit
        if(current->mnemonic[0] == '+') {
            format = 4;
            e = 1;
            memmove(&opcode[0], &opcode[1], strlen(opcode)-1);
            opcode[strlen(opcode)-1] = '\0'; // + 제거
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
            i = 1; n = 1;
        }

        if(strcmp(opcode, "LDB") == 0) {
            base = symbol_search(symtab, operand);
        }
        else if(strcmp(opcode, "BASE") == 0){
            current->obj_code = NONE;
            current = current->next;
            continue;
        }

        hash_node* opcode_node;

        if(strcmp(opcode, "START") == 0) {
            current->obj_code = NONE;
            current = current->next;
            continue;
        }
        else if((opcode_node = opcode_search(optab, opcode)) == NULL) {
            if(strcmp(opcode, "BYTE") == 0) {
                is_opcode_byte = 1;
                if(operand[0] == 'C') {
                    for(int j=2; j<strlen(operand)-1; j++) {
                        target_addr += (int)operand[j];
                        target_addr = target_addr << 8;
                    }
                    target_addr = target_addr >> 8;
                }
                else if(operand[0] == 'X') {
                    memmove(&operand[0], &operand[2], strlen(operand)-2);
                    operand[strlen(operand)-3] = '\0';
                    for (int j = 0; j < strlen(operand); ++j) {
                        if(operand[j] > 'F' || operand[j] < '0') {
                            return FAIL;
                        }
                    }
                    target_addr = (int)strtol(operand, NULL, 16);
                }
            }
            else {
                current->obj_code = NONE;
                current = current->next;
                continue;
            }
        }
        else { // asm 명령어인 경우
            obj_code = opcode_node->opcode;
            if((target_addr = symbol_search(symtab, operand)) == FAIL) { // symbol이 아님
                target_addr = (int)strtol(operand, NULL, 10);
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
                else {
                    current->mod_bit = 1;
                    b = 0; p = 0;
                    e = 1;
                }
            }
        }

        if(is_opcode_byte == 1) {
            obj_code = target_addr;
        }
        else {
            int xbpe = (x << 3) | (b << 2) | (p << 1) | e;

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

    if(write_lst_file(linelist, lst_file) == FAIL) {
        return FAIL;
    }
    if(write_obj_file(linelist, obj_file, prgm_len) == FAIL) {
        return FAIL;
    }

    return SUCCESS;
}



void line_list_init(line_list* list) {
    list->head = NULL;
    list->tail = NULL;
}
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

int write_lst_file(line_list* linelist, char* filename) {
    FILE* fp = fopen(filename, "w");
    if(fp == NULL) {
        printf("cannot open lst file to write\n");
        return FAIL;
    }
    line_node* current = linelist->head;

    while(current != NULL) {

        fprintf(fp, "%4d  ", current->line);
        if(current->LOCCTR != NONE) {
            fprintf(fp, "%04X    ", current->LOCCTR);
        }
        else {
            fprintf(fp, "%8s", "\0");
        }
        fprintf(fp, "%-8s  ", current->symbol);
        fprintf(fp, "%-8s  ", current->mnemonic);


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

        if(current->obj_code != NONE) {
            fprintf(fp, "%06X", current->obj_code);
        }

        fprintf(fp, "\n");
        current = current->next;
    }

    fclose(fp);
    return SUCCESS;
}

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
        if(strcmp(current->mnemonic, "START") == 0) {
            start_addr = (int)strtol(current->operand[0], NULL, 16);
            fprintf(fp, "H%-6s%06X%06X\n", current->symbol, start_addr, *prgm_len);
        }
        else if(strcmp(current->mnemonic, "END") == 0) {
            fprintf(fp, "%02X", byte_cnt);
            fprintf(fp, "%s\n", obj_code);
            strcpy(obj_code, "\0");
            mod_line_flag = 1;
            new_line_flag = 0;
        }
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
        else if(new_line_flag) {
            fprintf(fp, "T%06X", current->LOCCTR);
            line_start_addr = current->LOCCTR;
            new_line_flag = 0;
            continue;
        }
        else {
            if(current->mod_bit == 1) {
                mod_byte_array[mod_byte_cnt++] = current->LOCCTR;
            }
            if(current->obj_code <= 0xFF) {
                sprintf(obj_code, "%s%02X", obj_code, current->obj_code);
            }
            else if(current->obj_code <= 0xFFFF) {
                sprintf(obj_code, "%s%04X", obj_code, current->obj_code);
            }
            else {
                sprintf(obj_code, "%s%06X", obj_code, current->obj_code);
            }

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
            } else {
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
            else {
                next_bytes = node->LOCCTR - next_bytes;
            }

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
    if(mod_line_flag) {
        for (int i = 0; i < mod_byte_cnt; ++i) {
            fprintf(fp, "M%06X05\n", mod_byte_array[i]+1);
        }
    }
    fprintf(fp, "E%06X\n", start_addr);
    fclose(fp);
    return SUCCESS;
}