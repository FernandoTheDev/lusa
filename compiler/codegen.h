#pragma once

#include <stdint.h>

extern uint32_t bytecode[1024];
extern int bc_size;

extern char string_pool[100][100];
extern int string_count;

extern double float_pool[100];
extern int float_count;

void emit_instruction(uint8_t op, uint8_t rA, uint8_t rB, uint8_t rC);