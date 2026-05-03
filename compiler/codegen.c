#include "codegen.h"

uint32_t bytecode[1024]; // e se for maior?
int bc_size = 0;

// isso é TERRÍVEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEL
char string_pool[1000][1000]; // uso de memoria absurdamente elevado, 1000 * 1000 bytes = 1mb
int string_count = 0;

double float_pool[1000]; // uso de memoria absurdo, 1000 * 8 bytes = 8kb
int float_count = 0;

void emit_instruction(uint8_t op, uint8_t rA, uint8_t rB, uint8_t rC){
    uint32_t inst = (op << 24) | (rA << 16) | (rB << 8) | rC;
    bytecode[bc_size++] = inst;
}
