#pragma once

#include <stdlib.h>

typedef enum {
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
} opcode;

int vm_run(const char* filepath);