#pragma once

#include "../lexer.h"

typedef struct {
    Token current;
    Token previus;
    int hadError;
} Parser;

extern Parser parser;

void advance();
void consume(TokenType type, const char* message);

void expression(int target_reg);

void statement();