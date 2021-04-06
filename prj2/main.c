#include <stdio.h>
#include <string.h>
#include "stdlib.h"
int main() {
    char* temp = "F1";
    printf("%lu\n", strtol(temp, NULL, 16)/2);
}
int func(){
    FILE* mid_file = fopen("mid_file.txt", "w+");
    if(mid_file == NULL) {
        printf("cannot open file mid_file\n");
        return FAIL;
    }

    int start_addr;
    int LOCCTR = 0;

    while(1) { // 아직 중간 파일에 안 썼음
        char line[100];
        char line_token[3][30];
        int asm_tokens = 0;
        char* asm_ptr;
        char opcode[30];
        char operand[30];
        char operand2[30];

        fgets(line, 100, fp);
        if(feof(fp)) break;
        *lines+=5;

        int len = (int)strlen(line);
        line[len-1] = '\0';

        if(line[0] == '.') {
            fprintf(mid_file, ".\n");
            continue;
        }

        asm_ptr = strtok(line, " ");
        while(asm_ptr != NULL) {
            strcpy(line_token[asm_tokens], asm_ptr);
            asm_tokens++;
            asm_ptr = strtok(NULL, " ");
        }

        if(strcmp(line_token[0], "BASE") == 0) continue;

        if(asm_tokens == 3) { // label 있음
            if(opcode_search(optab, line_token[0]) == NULL) { // symbol 이라는 말
                strcpy(opcode, line_token[1]);
                strcpy(operand, line_token[2]);

                symbol_node* symbol = (symbol_node*)malloc(sizeof(symbol_node));
                strcpy(symbol->name, line_token[0]);
                symbol->LOCCTR = LOCCTR;
                if(symbol_search(symtab, symbol->name) != -1) {
                    *error_flag = 2;
                    break;
                }
                insert_sym(symtab, symbol);
                fprintf(mid_file, "%d\t%4X\t%s\t%s\t%s\n", *lines, LOCCTR, line_token[0], opcode, operand);

            }
            else { // operand 가 두개
                strcpy(opcode, line_token[0]);
                strcpy(operand, line_token[1]);
                strcpy(operand2, line_token[2]);
                fprintf(mid_file, "%d\t%4X\t-\t%s\t%s\t%s\n", *lines, LOCCTR, opcode, operand, operand2);

            }

        }
        else if(asm_tokens == 2) {
            strcpy(opcode, line_token[0]);
            strcpy(operand, line_token[1]);
            fprintf(mid_file, "%d\t%4X\t-\t%s\t%s\n", *lines, LOCCTR, opcode, operand);
        }
        else {
            strcpy(opcode, line_token[0]);
            fprintf(mid_file, "%d\t%4X\t-\t%s\t-\n", *lines, LOCCTR, opcode);
        }


        if (strcmp(opcode, "END") == 0) break;
        else if(strcmp(opcode, "START") == 0) {
            LOCCTR = (int)strtol(operand, NULL, 10);
            start_addr = LOCCTR;
        }
        else if(opcode[0] == '+') {
            LOCCTR += 4;
        }
        else {
            if(opcode_search(optab, opcode) != NULL || strcmp(opcode, "WORD") == 0) {
                LOCCTR += 3;
            }
            else if (strcmp(opcode, "RESW") == 0) {
                int operands_len = (int)strtol(operand, NULL, 10);
                LOCCTR += (3*operands_len);
            }
            else if (strcmp(opcode, "RESB") == 0) {
                int operands_len = (int)strtol(operand, NULL, 10);
                LOCCTR += operands_len;
            }
            else if (strcmp(opcode, "BYTE") == 0) {
                if(operand[0] == 'C') {
                    int operands_len = (int)strlen(operand);
                    LOCCTR += (operands_len-3);
                }
                else if (operand[0] == 'X') {
                    char* temp = operand;
                    strcpy(temp, temp+2);
                    strcpy(operand, temp);
                    operand[strlen(operand)-1] = '\0';
                    LOCCTR += (int)ceilf(strlen(operand)/2.0);
                }

            }
            else {
                *error_flag = 1;
                break;
            }
        }
    }
    *prgm_len = LOCCTR - start_addr;
    fclose(fp);
    fclose(mid_file);
    return SUCCESS;
};