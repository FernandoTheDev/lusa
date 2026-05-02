#pragma once 

#include <string.h>

typedef struct {
    char name[50];
    int reg_id;
    int tipo;
} symbol;

typedef struct {
    char name[50];
    int start_pc;
    int arity;
    int param_reg[25];
} Function;

extern Function func_table[256];
extern int func_counter;

extern symbol table[512];
extern int symbol_counter;
extern int next_reg_free;

int find_var(const char* name);
int var_register(const char* name, int tipo);