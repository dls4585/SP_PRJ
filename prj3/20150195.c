#include "20150195.h"


/* 전역변수 초기화 */
unsigned char memory[MAX_MEMORY_SIZE] = {0,};
int last_address = 0;
int PC = 0;
int CSADDR = 0;
int PROG_ADDRESS = 0;
int CSLTH = 0;

int main() {
    char cmd[MAX_CMD_LEN];
    char cmd_token[4][MAX_CMD_LEN];
    bucket* optab = (bucket*)malloc(sizeof(bucket) * HASH_SIZE);

    bucket* symtab = (bucket*)malloc(sizeof(bucket)*HASH_SIZE);
    bucket* recent_symtab = (bucket*)malloc(sizeof(bucket)*HASH_SIZE);

    bucket* estab = (bucket*)malloc(sizeof(bucket)*HASH_SIZE);

    symtab_init(recent_symtab);
    line_list* linelist = (line_list*)malloc(sizeof(line_list));
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

        // cmd == "quit" or "q"
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
                   "assemble filename\ntype filename\nsymbol\n");

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
                        line = i;
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
                        line = i;
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
            PC = (int)strtol(cmd_token[1], NULL, 16);
            if(PC > MAX_MEMORY_SIZE || PC < 0) {
                PC = 0;
                printf("wrong address, address must be between 0x00 ~ 0xFFFFF\n");
                clear(cmd, cmd_token, tokens);
                continue;
            }
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

            estab_init(estab);

            if(load_pass1(estab, cmd_token[1], cmd_token[2], cmd_token[3], files_num) == FAIL) {
                clear(cmd, cmd_token, tokens);
                continue;
            }

            for (int i = 0; i < HASH_SIZE; ++i) {
                printf("%d  : ", i);
                ES_node* current;
                int count = estab[i].count;
                for (current = estab[i].ES_head; current != NULL; current = current->next, count--) {
                    printf("[%X,%s]", current->address, current->name);
                    if(count != 1) {
                        printf("  ->  ");
                    }
                }
                printf("\n");
            }

            if(load_pass2(estab, cmd_token[1], cmd_token[2], cmd_token[3], files_num) == FAIL) {
                clear(cmd, cmd_token, tokens);
                continue;
            }

            // 명령어를 history 리스트에 추가
            Node* node = create_Node(cmd_token, tokens);
            list_push_back(history, node);
        }
            // 해당하는 명령어가 없는 경우 에러 처리
        else {
            printf("Wrong command format, use help for command information\n");
        }
        // 반복문마다 cmd와 cmd_token을 초기화 하고 stdin을 rewind한다.
        clear(cmd, cmd_token, tokens);
    }
}
