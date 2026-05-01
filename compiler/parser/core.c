#include <stdio.h>
#include "parser.h"

Parser parser;

void advance(){
    parser.previus = parser.current;
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
    printf("Erro de sintaxe: %s (Encontrado '%s')\n", message, parser.current.text);
    parser.hadError = 1;
}