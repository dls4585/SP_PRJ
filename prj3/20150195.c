#include "20150195.h"


/* 전역변수 초기화 */
unsigned char memory[MAX_MEMORY_SIZE] = {0,};
int last_address = 0;
int PC = 0;
int CSADDR = 0;
int PROG_ADDRESS = 0;
int CS_LENGTH = 0;
int PROG_LENGTH = 0;
int nextPC = 0;
int A = 0, X = 0, L = 0, S = 0, B = 0, T = 0;
char CC = '<';
int BP_flag = NO;

int main() {
    char cmd[MAX_CMD_LEN];
    char cmd_token[4][MAX_CMD_LEN];
    bucket* optab = (bucket*)malloc(sizeof(bucket) * HASH_SIZE);

    bucket* symtab = (bucket*)malloc(sizeof(bucket)*HASH_SIZE);
    bucket* recent_symtab = (bucket*)malloc(sizeof(bucket)*HASH_SIZE);

    bucket* estab = (bucket*)malloc(sizeof(bucket)*HASH_SIZE);

    symtab_init(recent_symtab);
    line_list* linelist = (line_list*)malloc(sizeof(line_list));

    // BP 용 리스트 추가
    int* BP_list = (int*)malloc(sizeof(int)*10);
    int BP_count = 0;
    int BP_max = 10;

    if(optab_init(optab) == FAIL) {
        printf("cannot initialize hash table\n");
        return 0;
    }

    List* history = (List*)malloc(sizeof(List));
    list_init(history);
    while (1) {
        int tokens = 0;
        printf("sicsim> ");
        fgets(cmd, MAX_CMD_LEN-1, stdin);
        cmd[strlen(cmd)-1] = '\0';

        replaceTab(cmd); // cmd 내의 \t 를 공백문자로 변환시켜준다.
        trim_cmd(cmd); // 명령어 좌 우 공백 다듬기

        /*
         ** tokenizing : 명령어와 인자 구분
         */
        char* ptr;
        char* temp = cmd;
        ptr=strsep(&temp, " ");
        strcpy(cmd_token[tokens], ptr);
        tokens++;
        int sep_success = 1;
        while((ptr=strsep(&temp,", ")) != NULL) {
            strcpy(cmd_token[tokens], ptr);
            if(temp != NULL) trim_cmd(temp);
            trim_cmd(cmd_token[tokens]);
            tokens++;
            // 최대로 들어올 수 있는 토큰이 4개이므로
            // ex. fill 3, 4, 7A
            // 4개 초과인 경우 실패
            if(tokens > 4) {
                sep_success = FAIL;
                break;
            }
        }

        /*
         ** 명령어의 유효성 검사
         */
        if(sep_success == FAIL) { // 토큰의 개수가 많아 tokenizing에 실패
            printf("Wrong command format, use help for command information\n");
            clear(cmd, cmd_token, tokens);
            continue;
        }
        if(is_lower(cmd_token[0]) == FAIL) { // 명령어가 소문자인지 검사
            printf("command must be lower case\n");
            clear(cmd, cmd_token, tokens);
            continue;
        }

        if(strcmp(cmd_token[0], "quit") == 0 || strcmp(cmd_token[0], "q") == 0) {
            if(!cmd_valid_check(tokens, QUIT)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);

            // 반복문 종료 -> 쉘 프로그램 종료
            break;
        }
            // cmd == "help" or "h"
        else if(strcmp(cmd_token[0], "help") == 0 || strcmp(cmd_token[0], "h") == 0) {
            if(!cmd_valid_check(tokens, HELP)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);

            // 명령어 리스트를 출력
            printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\n"
                   "du[mp] [start, end]\ne[dit] address, value\n"
                   "f[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\n"
                   "assemble filename\ntype filename\nsymbol\n"
                   "progaddr\nloader [object filename1] [object filename2] [object filename3]\n"
                   "bp [loc] or [clear]\nrun\n");
        }
            // cmd == "dir" or "d"
        else if(strcmp(cmd_token[0], "dir") == 0 || strcmp(cmd_token[0], "d") == 0) {
            if(!cmd_valid_check(tokens, DIRECTORY)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }

            // 현재 dir를 열어 읽는다
            DIR *dp = opendir(".");
            struct dirent *dent = readdir(dp);
            struct stat info;
            int ret = 0;

            while (dent) { // 더이상 읽을 파일이 없을 때까지(dent != NULL)
                char path[1024];
                sprintf(path, "./%s", dent->d_name);
                // ./FILENAME의 이름을 갖는 파일의 정보를 읽어들임
                ret = stat(path, &info);
                if(ret == FAIL) {
                    printf("%s", strerror(errno));
                    break;
                }
                // 파일이 directory인 경우 마지막에 /을 붙여 파일이름 출력
                if(S_ISDIR(info.st_mode)) {
                    printf("%s/\n", dent->d_name);
                }
                    // 실행가능 파일인 경우 *를 붙여 파일이름 출력
                else if(info.st_mode & S_IXUSR) {
                    printf("%s*\n", dent->d_name);
                }
                    // 이외의 파일은 이름만을 출력
                else {
                    printf("%s\n", dent->d_name);
                }
                dent = readdir(dp);
            }
            if(ret == FAIL) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);

        }
            // cmd == "history" or "hi"
        else if(strcmp(cmd_token[0], "history") == 0 || strcmp(cmd_token[0], "hi") == 0) {
            if(!cmd_valid_check(tokens,HISTORY)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);

            // history 리스트를 순회하며 출력한다.
            Node *current;
            int j = 1;
            for (current = history->head; current != NULL; current = current->next, ++j) {
                printf("%5d %s\n", j, current->cmd_history);
            }
        }
            // cmd == "dump" or "du"
        else if(strcmp(cmd_token[0], "dump") == 0 || strcmp(cmd_token[0], "du") == 0) {
            if(!cmd_valid_check(tokens,DUMP)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // start 와 end 인자 없이 dump만을 수행
            if(tokens == 1) {
                int i, line = last_address, end;
                // 마지막으로 출력한 주소에서 +160의 메모리 주소가
                // 가상메모리의 범위를 초과하는 경우 범위의 끝까지만 출력하도록
                // 아닌 경우 160개의 메모리 주소를 출력
                if(last_address + 160 > 0xFFFFF) {
                    end = 0x100000;
                }
                else {
                    end = last_address + 160;
                }
                // last_address ~ end의 메모리를 형식에 맞춰 출력한다
                for (i = last_address; i < end; ++i) {
                    if(i%16 == 0) {
                        printf("%05X ", i);
                    }
                    printf("%02X ", memory[i]);

                    // 한 line에 16개의 메모리를 출력하므로
                    // line의 마지막 주소까지 출력한 후 해당 메모리의 값을 출력한다
                    if(i%16 == 15) {
                        printf("; ");
                        for (int j = line; j < line + 16; ++j) {
                            // 메모리의 값의 ascii 값이 20~126의 범위 내라면 값을 출력
                            // 아니라면 .을 출력
                            if(memory[j] > 126 || memory[j] < 0x20) {
                                printf(".");
                            } else {
                                printf("%c", memory[j]);
                            }
                        }
                        printf("\n");
                        line = i+1;
                        // line을 i로 업데이트하여 다음 라인에서 다음 메모리 값 출력하도록 한다
                    }
                }
                last_address = i;
                // last_address 변수를 마지막으로 참조한 주소로 업데이트 한다
                if(last_address >= 0xFFFFF) {
                    last_address = 0;
                }
            }
                // dump의 인자가 2개 또는 3개인 경우
            else {
                // 인자를 16진수의 정수로 변환한다.
                int start = (int)strtol(cmd_token[1], NULL, 16), end;
                // start의 인자만 들어온 경우
                if(tokens == 2) {
                    if(args_check(cmd_token[1]) == FAIL) {
                        clear(cmd, cmd_token, tokens);
                        continue;
                    }
                    // end 값을 160개 또는 범위의 끝으로 지정한다.
                    if(start + 160 > 0xFFFFF) {
                        end = 0xFFFFF;
                    } else {
                        end = start + 159;
                    }
                }
                    // start와 end 인자가 들어온 경우
                else {
                    end = (int)strtol(cmd_token[2], NULL, 16);
                    if(args_check(cmd_token[1]) == FAIL || args_check(cmd_token[2]) == FAIL) {
                        clear(cmd, cmd_token, tokens);
                        continue;
                    }
                }
                // 인자가 메모리 내의 범위에 있는지 검사한다.
                if(start > 0xFFFFF || start < 0 || end > 0xFFFFF || end < 0) {
                    printf("arguments must be in range of 0x0 to 0xFFFFF.\n");
                    clear(cmd, cmd_token, tokens);
                    continue;
                }
                // start가 end보다 크다면 에러 처리한다.
                if(start > end) {
                    printf("START arguments must be lower than END argument.\n");
                    clear(cmd, cmd_token, tokens);
                    continue;
                }
                // start가 몇번째 line에 있는지 계산한다
                int line = (start/16)*16, i;
                // start가 line의 첫번째 주소가 아닌 경우
                // 이전 주소의 값은 공백으로 처리해야하므로
                // start를 16으로 남은 나머지 만큼 공백으로 출력
                if(start%16 != 0) {
                    printf("%05X ", line);
                    for (int j = start%16; j > 0; --j) {
                        printf("   ");
                    }
                }
                // 위와 동일
                for (i = start; i <= end; ++i) {
                    if(i%16 == 0) {
                        printf("%05X ", i);
                    }
                    printf("%02X ", memory[i]);
                    if(i%16 == 15) {
                        printf("; ");
                        for (int j = line; j < line + 16; ++j) {
                            if(j < start || memory[j] > 126 || memory[j] < 0x20) {
                                printf(".");
                            }
                            else {
                                printf("%c", memory[j]);
                            }
                        }
                        printf("\n");
                        line = i+1;
                    }
                    // line이 끝나지 않았으나, end에 도달한 경우
                    // 남은 만큼을 공백으로 처리하고
                    // ascii 값을 출력
                    if(i%16 != 15 && i == end) {
                        for (int j = 15 - (i % 16); j > 0; --j) {
                            printf("   ");
                        }
                        printf("; ");
                        for (int j = line; j < line + 16; ++j) {
                            if(j > i || memory[j] > 126 || memory[j] < 0x20) {
                                printf(".");
                            }
                            else {
                                printf("%c", memory[j]);
                            }
                        }
                        printf("\n");
                    }
                }
            }
            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);
        }
            // cmd == "edit" or e
        else if(strcmp(cmd_token[0], "edit") == 0 || strcmp(cmd_token[0], "e") == 0) {
            if(!cmd_valid_check(tokens, EDIT) || args_check(cmd_token[1]) == FAIL || args_check(cmd_token[2]) == FAIL) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // address 인자의 범위를 검사한다
            int address = (int)strtol(cmd_token[1], NULL, 16);
            unsigned int value = (unsigned int)strtol(cmd_token[2], NULL, 16);
            if(address > 0xFFFFF || address < 0) {
                printf("arguments must be in range of 0x0 to 0xFFFFF.\n");
                clear(cmd, cmd_token, tokens);
                continue;
            }

            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);

            // 해당 address의 값을 value 인자로 바꿔준다.
            memset(&memory[address], value, 1);
        }
            // cmd == "fill" or "f"
        else if(strcmp(cmd_token[0], "fill") == 0 || strcmp(cmd_token[0],"f") == 0) {
            if(!cmd_valid_check(tokens, FILL) || args_check(cmd_token[1]) == FAIL || args_check(cmd_token[2]) == FAIL || args_check(cmd_token[3]) == FAIL) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // 각 인자를 정수로 변환
            int start = (int)strtol(cmd_token[1], NULL, 16), end = (int)strtol(cmd_token[2], NULL, 16),
                    value = (int)strtol(cmd_token[3], NULL, 16);
            // start와 end의 범위를 검사한다.
            if(start > 0xFFFFF || start < 0 || end > 0xFFFFF || end < 0) {
                printf("arguments must be in range of 0x0 to 0xFFFFF.\n");
                clear(cmd, cmd_token, tokens);
                continue;
            }
            if(start > end) {
                printf("START arguments must be lower than END argument.\n");
                clear(cmd, cmd_token, tokens);
                continue;
            }

            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);

            // start~end 메모리의 값을 value로 바꿔준다.
            memset(&memory[start], value, end-start+1);
        }
            // cmd == "reset"
        else if(strcmp(cmd_token[0], "reset") == 0) {
            if(!cmd_valid_check(tokens, RESET)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);

            // 모든 메모리를 0으로 초기화시킨다.
            memset(memory, 0, MAX_MEMORY_SIZE);
        }
            // cmd == "opcode"
        else if(strcmp(cmd_token[0], "opcode") == 0) {
            if(!cmd_valid_check(tokens, MNEMONIC)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // mnemonic 인자가 소문자인 경우 에러 처리
            if(is_upper(cmd_token[1]) == FAIL) {
                printf("MNEMONIC must be UPPER case\n");
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // hash table에서 mnemonic 인자에 해당하는 opcode를 찾는다
            // 실패 시 NULL 반환
            hash_node* opcode_node = search_opcode(optab, cmd_token[1]);
            if(opcode_node == NULL) {
                printf("cannot find opcode_node of %s\n", cmd_token[1]);
                clear(cmd, cmd_token, tokens);
                continue;
            }

            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);

            printf("opcode_node is %X\n", opcode_node->opcode);

        }
            // cmd == "opcodelist"
        else if(strcmp(cmd_token[0], "opcodelist") == 0) {
            if(!cmd_valid_check(tokens,OPLIST)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);

            // hashtable을 순회하며 형식에 맞춰 opcodelist를 출력한다.
            for (int i = 0; i < HASH_SIZE; ++i) {
                printf("%d  : ", i);
                hash_node* current;
                int count = optab[i].count;
                for (current = optab[i].head; current != NULL; current = current->next, count--) {
                    printf("[%X,%s]", current->opcode, current->mnemonic);
                    if(count != 1) {
                        printf("  ->  ");
                    }
                }
                printf("\n");
            }
        }
            // cmd == "type"
        else if(strcmp(cmd_token[0], "type") == 0) {
            if(!cmd_valid_check(tokens,TYPE)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            struct stat info;
            int ret;

            char path[1024];
            sprintf(path, "./%s", cmd_token[1]);

            // 현재 디렉토리에서 인자로 들어온 파일 이름과 일치하는 파일을 찾는다.
            ret = stat(path, &info);
            if(ret == FAIL) {
                printf("cannot find file named %s\n", cmd_token[1]);
                clear(cmd, cmd_token, tokens);
                continue;
            }

            // 해당 파일을 연다.
            FILE* fp = fopen(cmd_token[1], "r");
            if(fp == NULL) {
                printf("cannot open file %s\n", cmd_token[1]);
                clear(cmd, cmd_token, tokens);
                continue;
            }

            // 해당 파일을 한 줄씩 출력한다.
            while(1) {
                char line[100];
                fgets(line, 100, fp);
                if(feof(fp)) break;

                int len = (int)strlen(line);
                line[len-1] = '\0';

                printf("%s\n", line);
            }
            fclose(fp);
            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);
        }
            // cmd == "assemble"
        else if (strcmp(cmd_token[0], "assemble") == 0) {
            if(!cmd_valid_check(tokens, ASSEMBLE)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // 확장자명 예외 처리
            char extension[4];
            char filename[30];
            memmove(&extension, &cmd_token[1][strlen(cmd_token[1])-4], 4); // 뒤 확장자 부분만 잘라낸다.
            strcpy(filename, cmd_token[1]);
            filename[strlen(filename)-4] = '\0';

            if(strcmp(extension, ".asm") != 0) {
                printf("extension for assemble must be .asm\n");
                clear(cmd,cmd_token,tokens);
                continue;
            }

            int error_flag = 0;
            int lines = 0;
            int prgm_len = 0;

            // assemble 명령어가 정상적으로 동작하기 시작했을 때 필요한 자료구조를 초기화한다.
            symtab_init(symtab);
            line_list_init(linelist);

            // pass 1
            if(pass1(cmd_token[1], optab, symtab, linelist, &lines, &prgm_len, &error_flag) == FAIL) {
                // pass 1이 실패 했을 경우 해당 라인에서 실패한 이유를 출력한다.
                if(error_flag == 1) {
                    printf("error in line #%d\n", lines);
                }
                else if (error_flag == 2) {
                    printf("duplicate symbol in line #%d\n", lines);
                }
                clear(cmd, cmd_token, tokens);
                continue;
            }

            // pass 2
            if(pass2(cmd_token[1], optab, symtab, linelist, &prgm_len) == FAIL) {
                printf("pass2 failed\n");
                clear(cmd, cmd_token, tokens);
                continue;
            }

            /*
             * 성공한 symbol table을 저장
             */
            for (int i = 0; i < HASH_SIZE; ++i) {
                recent_symtab[i] = symtab[i];
            }

            printf("[%s.lst], [%s.obj]\n", filename, filename);

            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);
        }
            // cmd == "symbol"
        else if(strcmp(cmd_token[0], "symbol") == 0) {
            if(!cmd_valid_check(tokens, SYMBOL)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }

            // symbol의 개수를 세어 개수만큼 동적 배열 할당
            int count = 0;
            for(int i=0; i < HASH_SIZE; i++) {
                count += recent_symtab[i].count;
            }

            if(count == 0) {
                printf("no symbol table, please assemble first\n");
                Node* node = create_Node(cmd_token, tokens);
                list_push_back(history, node);
                clear(cmd, cmd_token, tokens);
                continue;
            }

            char** symbols_name = (char**)malloc(sizeof(char*) * count);
            for (int i = 0; i < count; ++i) {
                symbols_name[i] = (char*)malloc(sizeof(char)*30);
            }

            // 동적할당한 배열에 symbol의 이름을 복사
            int j = 0;
            for (int i = 0; i < HASH_SIZE; ++i) {
                symbol_node* current = recent_symtab[i].s_head;
                for (; current != NULL ; current = current->next) {
                    strcpy(symbols_name[j++], current->name);
                }
            }

            // 배열을 오름차순으로 정렬
            char temp_str[30];
            for (int i = 0; i < count; ++i) {
                for (int k = 0; k < count - i - 1; ++k) {
                    if(strcmp(symbols_name[k], symbols_name[k+1]) > 0) {
                        strcpy(temp_str, symbols_name[k]);
                        strcpy(symbols_name[k], symbols_name[k+1]);
                        strcpy(symbols_name[k+1], temp_str);
                    }
                }
            }

            // symbol의 이름과 해당하는 주소값을 출력한다.
            for (int i = 0; i < count; ++i) {
                printf("\t%s\t%04X\n", symbols_name[i], search_symbol(recent_symtab, symbols_name[i]));
            }

            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);
        }
            // cmd == "progaddr"
        else if(strcmp(cmd_token[0], "progaddr") == 0) {
            if(!cmd_valid_check(tokens, PROGADDR)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // 인자를 16진수 정수로 변환하여 PC에 저장한다.
            PC = (int)strtol(cmd_token[1], NULL, 16);
            // PC의 범위(인자의 범위)를 검사한다.
            if(PC > MAX_MEMORY_SIZE || PC < 0) {
                PC = 0;
                printf("wrong address, address must be between 0x00 ~ 0xFFFFF\n");
                clear(cmd, cmd_token, tokens);
                continue;
            }
            // 프로그램 시작 주소를 인자로 받은 값으로 지정한다.
            PROG_ADDRESS = PC;

            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);
        }
            // cmd == "loader"
        else if(strcmp(cmd_token[0], "loader") == 0) {
            if(!cmd_valid_check(tokens, LOADER)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            int files_num = tokens - 1;
            int es_count = 0;
            PROG_LENGTH = 0;

            estab_init(estab);

            // linking loader 2-pass 알고리즘의 pass1을 수행한다.
            if(load_pass1(estab, cmd_token[1], cmd_token[2], cmd_token[3], files_num) == FAIL) {
                clear(cmd, cmd_token, tokens);
                continue;
            }

            // ESTAB에 저장된 ES의 개수를 센다.
            for (int i = 0; i < HASH_SIZE; ++i) {
                es_count += estab[i].count;
            }

            // ESTAB을 정렬하기 위해 sorted_estab에 복사해 저장한다.
            ES_node** sorted_estab = (ES_node**)malloc(sizeof(ES_node)*es_count);
            int j = 0;
            for(int i = 0; i < HASH_SIZE; ++i) {
                ES_node* current = estab[i].ES_head;
                for(; current != NULL; current = current->next) {
                    sorted_estab[j++] = current;
                }
            }

            // ES 배열을 주소 기준 오름차순으로 정렬
            for (int i = 0; i < es_count; ++i) {
                for (int k = 0; k < es_count - i - 1; ++k) {
                    if(sorted_estab[k]->address > sorted_estab[k + 1]->address) {
                        ES_node *temp_node = sorted_estab[k + 1];
                        sorted_estab[k + 1] = sorted_estab[k];
                        sorted_estab[k] = temp_node;
                    }
                }
            }

            // linking loader 2-pass 알고리즘의 pass2를 실행한다.
            if(load_pass2(estab, cmd_token[1], cmd_token[2], cmd_token[3], files_num) == FAIL) {
                clear(cmd, cmd_token, tokens);
                continue;
            }

            // LOAD MAP을 출력한다.
            printf("control symbol  address length\n");
            printf("section name\n");
            for (int i = 0; i < 32; ++i) {
                printf("- ");
            }
            printf("\n");

            for (int i = 0; i < es_count; ++i) {
                if(sorted_estab[i]->is_CSEC == YES) {
                    printf("%6s\t      \t%04X\t%04X\n", sorted_estab[i]->name, sorted_estab[i]->address, sorted_estab[i]->length);
                    PROG_LENGTH += sorted_estab[i]->length;
                }
                else {
                    printf("      \t%6s\t%04X\n", sorted_estab[i]->name, sorted_estab[i]->address);
                }
            }

            for (int i = 0; i < 32; ++i) {
                printf("- ");
            }
            printf("\n");
            printf("\t\ttotal length\t%04X\n", PROG_LENGTH);

            // 각 레지스터의 값을 초기화한다.
            A = 0, X = 0, B = 0, S = 0, T = 0, L = PROG_LENGTH;
            PC = PROG_ADDRESS;

            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);
        }
            // cmd == "bp"
        else if(strcmp(cmd_token[0], "bp") == 0) {
            if(!cmd_valid_check(tokens, BP)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            if(tokens == 2) { // BP와 함께 인자가 들어왔을 경우
                // 인자가 clear라면
                if(strcmp(cmd_token[1], "clear") == 0) {
                    // 모든 BP를 없애고
                    for (int i = 0; i < BP_count; ++i) {
                        BP_list[i] = NO;
                    }
                    // BP_count를 0으로 초기화한다.
                    BP_count = 0;
                    printf("[");
                    printf(CYN "ok");
                    printf(CRESET "] clear all breakpoints\n");
                }
                else {
                    // 인자가 LOC으로 들어온 경우
                    int arg = (int)strtol(cmd_token[1], NULL, 16);
                    if(arg > PROG_LENGTH || arg < 0) {
                        printf("bp out of program. bp must be in range of PROG_LENGTH.\n");
                        clear(cmd, cmd_token, tokens);
                        continue;
                    }
                    // BP를 새롭게 저장했을 경우 BP_list에 저장할 수 없으면 새로운 크기의 배열을 할당받는다.
                    if(BP_count+1 > BP_max) {
                        int* backup = BP_list;
                        // 새로운 크기의 배열에 할당했으면 원래의 주소로 다시 되돌리고 에러 처리한다.
                        if((BP_list = (int*)realloc(BP_list, sizeof(int)*(BP_max + 10))) == NULL) {
                            printf("no more memory to add BP.\n");
                            BP_list = backup;
                            clear(cmd, cmd_token, tokens);
                            continue;
                        }
                        BP_max += 10;
                    }
                    // BP_list에 인자로 받은 값을 새로 저장한다.
                    BP_list[BP_count] = arg;

                    printf("[");
                    printf(CYN "ok");
                    printf(CRESET "] create breakpoint %X\n", BP_list[BP_count]);
                    BP_count++;
                }
            }
                // 인자로 아무것도 들어오지 않았을 경우
            else {
                // 모든 BP를 출력한다.
                printf("\t\tbreakpoint\n\t\t");
                for (int i = 0; i < 10; ++i) {
                    printf("- ");
                }
                printf("\n");
                for (int i = 0; i < BP_count; ++i) {
                    printf("\t\t%X\n", BP_list[i]);
                }
            }

            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);
        }
            // cmd == "run"
        else if(strcmp(cmd_token[0], "run") == 0) {
            if(!cmd_valid_check(tokens, RUN)) {
                clear(cmd, cmd_token, tokens);
                continue;
            }
            int result;

            /*
             * run을 수행한다.
             */

            // run의 결과가 FAIL인 경우값 에러 처리한다.
            if((result = run(BP_list, BP_count, optab)) == FAIL) {
                printf("running program stopped for some reason.\n");
                clear(cmd, cmd_token, tokens);
                continue;
            }
                // run이 모두 성공적으로 마쳤을 경우
            else if(result == SUCCESS) {
                // 레지스터의 현재 값을 출력한다.
                print_registers();
                printf("\t\tEnd Program\n");
            }
                // run이 BP에 의해 중단된 경우
            else if(result == PC) {
                printf("\t\tStop at checkpoint[%X]\n", PC-PROG_ADDRESS);
            }

            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);
        }
            // 해당하는 명령어가 없는 경우 에러 처리
        else {
            printf("Wrong command format, use help for command information\n");
        }
        // cmd == "quit" or "q"
        // 반복문마다 cmd와 cmd_token을 초기화 하고 stdin을 rewind한다.
        clear(cmd, cmd_token, tokens);
    }
}
