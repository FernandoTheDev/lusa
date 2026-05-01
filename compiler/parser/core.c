#include <stdio.h>
#include "parser.h"

Parser parser;

void advance(){
    parser.previous = parser.current;
    parser.current = next_token();

    while(parser.current.type == TK_ERROR){
        printf("Erro Lexico: %s\n", parser.current.text);
        parser.current = next_token();
    }
}

void consume(TokenType type, const char* message){
    if (parser.current.type == type){
        advance();
        return;
    }
    if (type == TK_SEMICOLON || type == TK_RPAREN || type == TK_RBRACKET || type == TK_RBRACE) {
         printf("\033[1;31m[ERRO]\033[0m %s:%d:%d \n -> %s\n -> Logo apos: '%s'\n", parser.filepath, parser.previous.line, parser.previous.col,  message, parser.previous.text);
    } else {
        printf("\033[1;31m[ERRO]\033[0m %s:%d:%d \n -> %s\n -> Encontrado: '%s'\n", parser.filepath, parser.current.line, parser.current.col, message, parser.current.text);
    }
    parser.hadError = 1;
}