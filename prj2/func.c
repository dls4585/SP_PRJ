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
int make_opcode_table(bucket* hashtable) {
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

        insert_opcode(hashtable, hash, OPCODE);
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
int opcode_search(bucket* hashtable, char* mnemonic) {
    int hash_index = hash_function(mnemonic);
    hash_node* current = hashtable[hash_index].head;
    for (; current != NULL; current = current->next) {
        if(strcmp(current->mnemonic, mnemonic) == 0)
            return current->opcode;
    }
    return -1;
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

void make_symbol_table(bucket* symtab) {
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
    return -1;
}