#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"
#include "lusa_string.h"

static char source[10240];
static int counter = 0;
static int current_line = 1;
static int current_col = 1;

void init_lexer(const char* source_code){
    lusa_strcpy(source, sizeof(source), source_code);
    counter = 0;
    current_line = 1;
    current_col = 1;
}

Token next_token(){
    Token token;
    token.text[0] = '\0';

    while(1){
        if (isspace(source[counter])){
            if (source[counter] == '\n'){
                current_line++;
                current_col = 1;
            } else {
                current_col++;
            }
            counter ++;
        }

        else if (source[counter] == '/' && source[counter + 1] == '/'){
            while(source[counter] != '\n' && source[counter] != '\0'){
                counter ++;
                current_col++;
            }
        } else {
            break;
        }
    }

    char code = source[counter];

    token.line = current_line;
    token.col = current_col;

    switch (code)
    {
    case '\0':
        token.type = TK_EOF;
        return token;
    case '=':
        if (source[counter + 1] == '='){
            token.type = TK_EQEQ;
            lusa_strcpy(token.text, sizeof(token.text), "==");
            counter += 2;
            current_line += 2;
        } else {
            token.type = TK_EQUAL;
            lusa_strcpy(token.text, sizeof(token.text), "=");
            counter++;
            current_col++;
        }
        return token;
    case '"': {
        counter ++;
        current_col++;
        int i = 0;
        while(source[counter] != '"' && source[counter] != '\0'){
            token.text[i++] = source[counter++];
            current_col++;
        }
        token.text[i] = '\0';

        if(source[counter] == '"') {
            counter++;
            current_col++;
        }

        token.type = TK_STRING;
        return token;
    }
    case '<':
        token.type = TK_LT;
        lusa_strcpy(token.text, sizeof(token.text), "<");
        counter ++;
        current_col++;
        return token;
    case '>':
        token.type = TK_GT;
        lusa_strcpy(token.text, sizeof(token.text), ">");
        counter++;
        current_col++;
        return token;
    case '+':
        token.type = TK_PLUS;
        lusa_strcpy(token.text, sizeof(token.text), "+");
        counter++;
        current_col++;
        return token;
    case '-':
        token.type = TK_MINUS;
        lusa_strcpy(token.text, sizeof(token.text), "-");
        counter++;
        current_col++;
        return token;
    case ';':
        token.type = TK_SEMICOLON;
        lusa_strcpy(token.text, sizeof(token.text), ";");
        counter ++;
        current_col++;
        return token;
    case '{':
        token.type = TK_LBRACE;
        lusa_strcpy(token.text, sizeof(token.text), "{");
        counter++;
        current_col++;
        return token;
    case '}':
        token.type = TK_RBRACE;
        lusa_strcpy(token.text, sizeof(token.text), "}");
        counter++;
        current_col++;
        return token;
    case '(':
        token.type = TK_LPAREN;
        lusa_strcpy(token.text, sizeof(token.text), "(");
        counter++;
        current_col++;
        return token;
    case ')':
        token.type = TK_RPAREN;
        lusa_strcpy(token.text, sizeof(token.text), ")");
        counter++;
        current_col++;
        return token;
    case ',':
        token.type = TK_COMMA;
        lusa_strcpy(token.text, sizeof(token.text), ",");
        counter++;
        current_col++;
        return token;
    case '[':
        token.type = TK_LBRACKET;
        lusa_strcpy(token.text, sizeof(token.text), "[");
        counter++;
        current_col++;
        return token;
        
    case ']':
        token.type = TK_RBRACKET;
        lusa_strcpy(token.text, sizeof(token.text), "]");
        counter++;
        current_col++;
        return token;
    }
    

    if (isdigit(code)){
        int i = 0;
        int isFloat = 0;
        while(isdigit(source[counter]) || source[counter] == '.'){
            if (source[counter] == '.'){
                if (isFloat >= 1){
                    token.type = TK_ERROR;
                    lusa_strcpy(token.text, sizeof(token.text), "Numero com multiplos pontos decimais");
                    return token;
                } else {
                isFloat = 1;
            }
            }
            token.text[i++] = source[counter++];
            current_col++;
        }
        token.text[i] = '\0';

        token.type = isFloat ? TK_FLOAT : TK_INT;

        return token;
    }

    if (isalpha(code) || code == '_' || code == '-' || (unsigned char)code >= 128){
        int i = 0;
        while(isalnum(source[counter]) || source[counter] == '_' || source[counter] == '-' || (unsigned char)source[counter] >= 128){
            token.text[i++] = source[counter++];
            current_col++;
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
    current_col++;
    return token;
}