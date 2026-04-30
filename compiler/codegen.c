#include "codegen.h"

uint32_t bytecode[1024];
int bc_size = 0;

char string_pool[100][100];
int string_count = 0;

void emit_instruction(uint8_t op, uint8_t rA, uint8_t rB, uint8_t rC){
    uint32_t inst = (op << 24) | (rA << 16) | (rB << 8) | rC;
    bytecode[bc_size++] = inst;
}