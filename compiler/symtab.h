#pragma once

#include <string.h>

typedef struct
{
    char name[50]; // e se for maior?
    // o uso de uint8 ao inves de int é melhor, registradores e tipos nunca serão valor negativo e nem passarão de 512
    // voce pode aumentar caso precise
    __uint8_t reg_id;
    __uint8_t tipo;
} symbol;

typedef struct
{
    char name[50]; // e se for maior?
    int start_pc; // mesma coisa do symbol
    int arity; // mesma coisa do symbol
    int param_reg[25]; // mesma coisa do symbol
} Function;

extern Function func_table[256]; // e se for maior?
extern int func_counter;

extern symbol table[512]; // e se for maior?
extern int symbol_counter;
extern int next_reg_free;

int find_var(const char *name);
int var_register(const char *name, int tipo);
