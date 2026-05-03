#pragma once

#include <stdlib.h>
#include "lusa_utils.h"

typedef enum
{
    HALT,
    LOAD,
    MOV,
    ADD,
    FADD,
    SUB,
    FSUB,
    EQ,
    CMP,
    LT,
    GT,
    JMP,
    JMPF,
    CALL,
    CALL_EXT,
    RET,
    LOAD_STR,
    LOAD_FLT,
    ALLOC,
    STORE,
    PUSH,
    POP,
    READ,
    SCAT,
} opcode;

int vm_execute_module(LusaModule *module);
void vm_execute_from_buffer(unsigned char *buffer, uint32_t size);
void free_lusa_module(LusaModule *module);
int vm_run(const char *filepath);
