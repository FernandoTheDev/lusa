#include <stdio.h>
#include <ctype.h>

#include "compiler.h"
#include "../lvm/lvm.h"
#include "lexer.h"
#include "symtab.h"
#include "codegen.h"
#include "parser/parser.h"
#include "lusa_utils.h"

int compile(const char* in_filepath, const char* out_filepath){
    FILE* file_in;
    if(lusa_fopen(&file_in, in_filepath, "r") != 0 || file_in == NULL){
        printf("[COMPILADOR] ERRO: Nao abriu %s\n", in_filepath);
        return -1;
    }
    char temp_source[10240];
    size_t n = 0;
    int code;
    while ((code = fgetc(file_in)) != EOF && n < sizeof(temp_source) - 1){
        temp_source[n++] = (char)code;
    }
    temp_source[n] = '\0';
    fclose(file_in);

    init_lexer(temp_source);
    bc_size = 0;
    string_count = 0;
    symbol_counter = 0;
    next_reg_free = 0;
    func_counter = 0;
    parser.hadError = 0;

    advance();

    while(parser.current.type != TK_EOF){
        statement();
    }

    emit_instruction(HALT, 0, 0, 0);

    FILE* file_out;
    if(lusa_fopen(&file_out, out_filepath, "wb") == 0 && file_out != NULL){
        fwrite(&bc_size, sizeof(int), 1, file_out);
        fwrite(bytecode, sizeof(uint32_t), bc_size, file_out);
        fwrite(&string_count, sizeof(int), 1, file_out);
        fwrite(string_pool, sizeof(char) * 100, string_count, file_out);
        fclose(file_out);
    }


    return parser.hadError == 0 ? 0 : -1;
}