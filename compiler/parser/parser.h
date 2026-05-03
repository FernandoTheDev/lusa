#pragma once

#include "../lexer.h"

typedef struct
{
    Token current;
    Token previous;
    int hadError;
    char filepath[256];
} Parser;

extern Parser parser;

void advance();
void consume(TokenType type, const char *message);

TokenType expression(int target_reg);

void statement();
