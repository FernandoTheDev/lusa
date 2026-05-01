#pragma once

#include <stdint.h>

typedef enum {
    TK_EOF,
    TK_VAR,
    TK_ID,
    TK_NUMBER,
    TK_INT,
    TK_FLOAT,
    TK_STRING,
    TK_BOOL,
    TK_TRUE,
    TK_FALSE,
    TK_EQUAL,
    TK_EQEQ,
    TK_LT,
    TK_GT,
    TK_PLUS,
    TK_MINUS,
    TK_SEMICOLON,
    TK_TYPE,
    TK_ERROR,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
    TK_FN,
    TK_RETURN,
    TK_LBRACE,
    TK_RBRACE,
    TK_LPAREN,
    TK_RPAREN,
    TK_COMMA,
    TK_LBRACKET,
    TK_RBRACKET,
} TokenType;

typedef struct {
    TokenType type;
    char text[100];
} Token;

void init_lexer(const char* source_code);

Token next_token();