#pragma once

#include <stdint.h>

typedef enum
{
    TK_EOF,
    TK_VAR,
    TK_ID,
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
    TK_ARRAY,
} TokenType;

typedef struct
{
    TokenType type;
    char text[100]; // esquece isso, 100 bytes atoa
    int line;
    int col;
} Token;

typedef struct {
    char* data; // cria um slice diretamente do source code, não tem copia nem nada, é bem melhor
    uint16_t len; // nenhuma string vai ter mais de 65 mil caracteres, é loucura
} LusaStr;

// essa estrutura nova gasta muito menos memoria por token, alem de ter valores especializados pra cada tipo por padrão
typedef struct {
    TokenType type;
    union {
        LusaStr str;
        long integer;
        double d;
        float f;
        char ch;
    };
    uint32_t line;
    uint32_t col;
} Token;

void init_lexer(const char *source_code);

Token next_token();
