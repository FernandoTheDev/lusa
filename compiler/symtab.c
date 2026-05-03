#include <stdio.h>
#include "symtab.h"
#include "lusa_string.h"

Function func_table[256];
int func_counter = 0;

symbol table[512];
int symbol_counter = 0;
int next_reg_free = 1;

int find_var(const char *name)
{
    for (int i = 0; i < symbol_counter; i++)
    {
        if (strcmp(table[i].name, name) == 0)
        {
            return table[i].reg_id;
        }
    }
    return -1;
}

int var_register(const char *name, int tipo)
{
    if (symbol_counter >= 256)
    {
        printf("[COMPILADOR] ERRO: Limite de variaveis antigido.\n");
        return -1;
    }
    if (find_var(name) != -1)
    {
        printf("[COMPILADOR] ERRO: Variavel '%s' ja foi declarada.\n", name);
        return -1;
    }

    lusa_strcpy(table[symbol_counter].name, 50, name);
    table[symbol_counter].reg_id = next_reg_free;
    table[symbol_counter].tipo = tipo;

    symbol_counter++;
    return next_reg_free++;
}
