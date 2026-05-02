#include <stdio.h>
#include <ctype.h>

#include "compiler.h"
#include "../lvm/lvm.h"
#include "lexer.h"
#include "symtab.h"
#include "codegen.h"
#include "parser/parser.h"
#include "lusa_string.h"

LusaModule* compile_to_module(const char* in_filepath){
    FILE* file_in;
    if(lusa_fopen(&file_in, in_filepath, "r") != 0 || file_in == NULL){
        printf("\033[1;31mERRO:[0m\033 Nao abriu %s\n", in_filepath);
        return NULL;
    }

    char temp_source[10240];
    size_t n = 0;
    int code;
    while((code = fgetc(file_in)) != EOF && n < sizeof(temp_source) - 1){
        temp_source[n++] = (char)code;
    }
    temp_source[n] = '\0';
    fclose(file_in);

    init_lexer(temp_source);
    bc_size = 0;
    string_count = 0;
    float_count = 0;
    symbol_counter = 0;
    next_reg_free = 0;
    func_counter = 0;
    parser.hadError = 0;
    lusa_strcpy(parser.filepath, sizeof(parser.filepath), in_filepath);

    advance();
    while(parser.current.type != TK_EOF){
        statement();
    }
    emit_instruction(HALT, 0, 0, 0);

    if(parser.hadError) return NULL;

    LusaModule* module = (LusaModule*)malloc(sizeof(LusaModule));

    module->code_size = bc_size;
    module->code = (uint32_t*)malloc(sizeof(uint32_t) * bc_size);
    memcpy(module->code, bytecode, sizeof(uint32_t) * bc_size);

    module->string_count = string_count;
    module->strings = (char (*)[1000])malloc(sizeof(char[1000]) * string_count);
    memcpy(module->strings, string_pool, sizeof(char[1000]) * string_count);

    module->float_count = float_count;
    if(float_count > 0) {
        module->floats = (double*)malloc(sizeof(double) * float_count);
        memcpy(module->floats, float_pool, sizeof(double) * float_count);
    } else {
        module->floats = NULL;
    }
    return module;
}

int build_executable(const char* current_exe, const char* bc_file, const char* out_exe){
    FILE* f_exe;
    if(lusa_fopen(&f_exe, current_exe, "rb") != 0 || !f_exe){
        printf("ERRO: Nao foi possivel ler o compilador base '%s'.\n", current_exe);
        return -1;
    }

    FILE* f_bc;
    if (lusa_fopen(&f_bc, bc_file, "rb") != 0 || !f_bc){
        fclose(f_exe);
        return -1;
    }

    FILE* f_out;
    if (lusa_fopen(&f_out, out_exe, "wb") != 0 || !f_out){
        fclose(f_exe); fclose(f_bc);
        return -1;
    }

    char buffer[4096];
    size_t bytes;
    while((bytes = fread(buffer, 1, sizeof(buffer), f_exe)) > 0){
        fwrite(buffer, 1, bytes, f_out);
    }

    uint32_t bc_size = 0;
    while((bytes = fread(buffer, 1, sizeof(buffer), f_bc)) > 0){
        fwrite(buffer, 1, bytes, f_out);
        bc_size += (uint32_t)bytes;
    }

    uint32_t magic = LUSA_MAGIC;
    fwrite(&bc_size, sizeof(uint32_t), 1, f_out);
    fwrite(&magic, sizeof(uint32_t), 1, f_out);

    fclose(f_exe); fclose(f_bc); fclose(f_out);
    return 0;
}

int compile(const char* in_filepath, const char* out_filepath){
    LusaModule* module = compile_to_module(in_filepath);
    if (module == NULL) return -1;

    FILE* file_out;
    if (lusa_fopen(&file_out, out_filepath, "wb") == 0 && file_out != NULL){
        fwrite(&module->code_size, sizeof(int), 1, file_out);
        fwrite(module->code, sizeof(uint32_t), module->code_size, file_out);

        fwrite(&module->string_count, sizeof(int), 1, file_out);
        if(module->string_count > 0){
            fwrite(module->strings, sizeof(char[1000]), module->string_count, file_out);
        }

        fwrite(&module->float_count, sizeof(int), 1, file_out);
        if (module->float_count > 0){
            fwrite(module->floats, sizeof(double), module->float_count, file_out);
        }
        fclose(file_out);
    }

    free_lusa_module(module);
    return 0;
}