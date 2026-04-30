#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

static char source[10240];
static int counter = 0;

void init_lexer(const char* source_code){
    strcpy_s(source, sizeof(source), source_code);
    counter = 0;
}

Token next_token(){
    Token token;
    token.text[0] = '\0';

    while(1){
        if (isspace(source[counter])){
            counter ++;
        }

        else if (source[counter] == '/' && source[counter + 1] == '/'){
            while(source[counter] != '\n' && source[counter] != '\0'){
                counter ++;
            }
        } else {
            break;
        }
    }

    while(isspace(source[counter])){
        counter++;
    }

    char code = source[counter];

    switch (code)
    {
    case '\0':
        token.type = TK_EOF;
        return token;
    case '=':
        if (source[counter + 1] == '='){
            token.type = TK_EQEQ;
            strcpy_s(token.text, sizeof(token.text), "==");
            counter += 2;
        } else {
            token.type = TK_EQUAL;
            strcpy_s(token.text, sizeof(token.text), "=");
            counter++;
        }
        return token;
    case '"': {
        counter ++;
        int i = 0;
        while(source[counter] != '"' && source[counter] != '\0'){
            token.text[i++] = source[counter++];
        }
        token.text[i] = '\0';

        if(source[counter] == '"') counter++;

        token.type = TK_STRING;
        return token;
    }
    case '<':
        token.type = TK_LT;
        strcpy_s(token.text, sizeof(token.text), "<");
        counter ++;
        return token;
    case '>':
        token.type = TK_GT;
        strcpy_s(token.text, sizeof(token.text), ">");
        counter++;
        return token;
    case '+':
        token.type = TK_PLUS;
        strcpy_s(token.text, sizeof(token.text), "+");
        counter++;
        return token;
    case '-':
        token.type = TK_MINUS;
        strcpy_s(token.text, sizeof(token.text), "-");
        counter++;
        return token;
    case ';':
        token.type = TK_SEMICOLON;
        strcpy_s(token.text, sizeof(token.text), ";");
        counter ++;
        return token;
    case '{':
        token.type = TK_LBRACE;
        strcpy_s(token.text, sizeof(token.text), "{");
        counter++;
        return token;
    case '}':
        token.type = TK_RBRACE;
        strcpy_s(token.text, sizeof(token.text), "}");
        counter++;
        return token;
    case '(':
        token.type = TK_LPAREN;
        strcpy_s(token.text, sizeof(token.text), "(");
        counter++;
        return token;
    case ')':
        token.type = TK_RPAREN;
        strcpy_s(token.text, sizeof(token.text), ")");
        counter++;
        return token;
    case ',':
        token.type = TK_COMMA;
        strcpy_s(token.text, sizeof(token.text), ",");
        counter++;
        return token;
    case '[':
        token.type = TK_LBRACKET;
        strcpy_s(token.text, sizeof(token.text), "[");
        counter++;
        return token;
    case ']':
        token.type = TK_RBRACKET;
        strcpy_s(token.text, sizeof(token.text), "]");
        counter++;
        return token;
    }
    

    if (isdigit(code)){
        int i = 0;
        while(isdigit(source[counter])){
            token.text[i++] = source[counter++];
        }
        token.text[i] = '\0';
        token.type = TK_NUMBER;
        return token;
    }

    if (isalpha(code) || code == '_' || code == '-'){
        int i = 0;
        while(isalnum(source[counter]) || source[counter] == '_' || source[counter] == '-'){
            token.text[i++] = source[counter++];
        }
        token.text[i] = '\0';

        if (strcmp(token.text, "var") == 0) token.type = TK_VAR;
        else if (strcmp(token.text, "int") == 0) token.type = TK_INT;
        else if (strcmp(token.text, "float") == 0) token.type = TK_FLOAT;
        else if (strcmp(token.text, "string") == 0) token.type = TK_STRING;
        else if (strcmp(token.text, "bool") == 0) token.type = TK_BOOL;
        else if (strcmp(token.text, "string") == 0) token.type = TK_STRING;
        else if (strcmp(token.text, "true") == 0) token.type = TK_TRUE;
        else if (strcmp(token.text, "false") == 0) token.type = TK_FALSE;
        else if(strcmp(token.text, "if") == 0) token.type = TK_IF;
        else if(strcmp(token.text, "else") == 0) token.type = TK_ELSE;
        else if(strcmp(token.text, "while") == 0) token.type = TK_WHILE;
        //else if(strcmp(token.text, "for") == 0) token.type = TK_FOR;
        else if(strcmp(token.text, "fn") == 0) token.type = TK_FN;
        else if(strcmp(token.text, "return") == 0) token.type = TK_RETURN;
        else token.type = TK_ID;

        return token;
    }

    token.type = TK_ERROR;
    token.text[0] = code;
    token.text[1] = '\0';
    counter++;
    return token;
}