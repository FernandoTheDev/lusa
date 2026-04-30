#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "../symtab.h"
#include "../codegen.h"
#include "../../lvm/lvm.h"

void expression(int target_reg);

void factor(int target_reg){
    switch(parser.current.type){
        case TK_NUMBER:{
            advance();
            int valor = atoi(parser.previus.text);
            uint8_t high = (valor >> 8) & 0xFF;
            uint8_t low = valor & 0xFF;
            emit_instruction(LOAD, target_reg, high, low);
            break;
        }
        case TK_ID:{
            advance();
            char target_name[50];
            strcpy_s(target_name, 50, parser.previus.text);

            if (parser.current.type == TK_LPAREN){
                if (strcmp(target_name, "print") == 0){
                    advance();
                    expression(0);
                    consume(TK_RPAREN, "esperava ')' apos print.");
                    emit_instruction(CALL_EXT, 0, 1, 0);
                    return;
                } else if(strcmp(target_name, "print_str") == 0) {
                    advance();
                    expression(0);
                    consume(TK_RPAREN, "esperava ')' apos o print_str");
                    emit_instruction(LOAD_STR, 0, 2, 0);
                    consume(TK_SEMICOLON, "Esperava ';' no final do print_str");
                    return;
                } else{
                    int dest_pc = -1;
                    int fn_index = -1;
                    for (int i = 0; i < func_counter; i++){
                        if(strcmp(func_table[i].name, target_name) == 0){ 
                            dest_pc = func_table[i].start_pc;
                            fn_index = i;
                        }
                    } 

                    if (dest_pc == -1){
                        printf("[COMPILADOR] ERRO: funcao '%s' nao existe.\n", target_name);
                        parser.hadError = 1;
                    } else {
                        int args_read = 0;
                        if(parser.current.type != TK_RPAREN){
                            do {
                                if (parser.current.type == TK_COMMA) advance();

                                if (args_read >= func_table[fn_index].arity){
                                    printf("[COMPILADOR] ERRO: Muitos argumentos passados para a funcao '%s'.\n", target_name);
                                    parser.hadError = 1;
                                    break;
                                }

                                int temp_arg_reg = next_reg_free++;
                                expression(temp_arg_reg);

                                int target_param_reg = func_table[fn_index].param_reg[args_read];
                                emit_instruction(MOV, target_param_reg, temp_arg_reg, 0);

                                next_reg_free--;
                                args_read++;
                            } while (parser.current.type == TK_COMMA);
                        } 

                        if (args_read < func_table[fn_index].arity){
                            printf("[COMPILADOR] ERRO: funcao '%s' esperava '%d' argumentos, recebeu %d.\n", target_name, func_table[fn_index].arity, args_read);
                            parser.hadError = 1;
                        }

                        consume(TK_RPAREN, "Esperava ')' apos chamada de funcao");

                        uint8_t rB = (dest_pc >> 8) & 0xFF;
                        uint8_t rC = dest_pc & 0xFF;
                        emit_instruction(CALL, 0, rB, rC);
                        emit_instruction(MOV, target_reg, 0, 0);
                    }
                }
            } else if(parser.current.type == TK_LBRACKET){
                advance();

                int index_reg = next_reg_free++;
                expression(index_reg);
                consume(TK_RBRACKET, "Esperava ']' apos o indice do array.");

                int reg_origem = find_var(target_name);

                if(reg_origem == -1){
                    printf("[COMPILADOR] ERRO: Variavel '%s' nao declarada.\n", target_name);
                    parser.hadError = 1;
                } else {
                    emit_instruction(READ, target_reg, reg_origem, index_reg);
                }
                next_reg_free--;
            } else {

                int reg_origem = find_var(parser.previus.text);

                if(reg_origem == -1){
                    printf("[COMPILADOR] ERRO: Variavel '%s' nao declarada.\n", parser.previus.text);
                    parser.hadError = 1;
                } else {
                    emit_instruction(MOV, target_reg, reg_origem, 0);
                }
            }
            break;
        }
        case TK_TRUE:
            advance();
            emit_instruction(LOAD, target_reg, 0, 1);
            break;
        case TK_FALSE:
            advance();
            emit_instruction(LOAD, target_reg, 0, 0);
            break;
        case TK_STRING:{
            advance();
            int str_idx = string_count++;
            strcpy_s(string_pool[str_idx], 100, parser.previus.text);
            
            uint8_t high = (str_idx >> 8) & 0xFF;
            uint8_t low = str_idx & 0xFF;
            emit_instruction(LOAD_STR, target_reg, high, low);
            break;
        }
        case TK_LPAREN: {
            advance();
            expression(target_reg);
            consume(TK_RPAREN, "Esperava ')' apos a expressao");
            break;
        }
        case TK_LBRACKET: {
            advance();

            int size_reg = next_reg_free++;
            int patch_pc = bc_size;
            emit_instruction(LOAD, size_reg, 0, 0);

            emit_instruction(ALLOC, target_reg, size_reg, 0);

            int count = 0;
            if (parser.current.type != TK_RBRACKET){
                do {
                    if (parser.current.type == TK_COMMA) advance();

                    int val_reg = next_reg_free++;
                    expression(val_reg);

                    int index_reg = next_reg_free++;
                    uint8_t high = (count >> 8) & 0xFF;
                    uint8_t low = count & 0xFF;
                    emit_instruction(LOAD, index_reg, high, low);

                    emit_instruction(STORE, val_reg, target_reg, index_reg);

                    next_reg_free -= 2;
                    count++;
                } while (parser.current.type == TK_COMMA);
            }
            consume(TK_RBRACKET, "Esperava ']' no final do array.");

            bytecode[patch_pc] = (LOAD << 24) | (size_reg << 16) | ((count >> 8) & 0xFF) << 8 | (count & 0xFF);

            next_reg_free--;
            break;
        }
        default:
            printf("[COMPILADOR] ERRO de sintaxe: esperava um numero ou variavel.");
            parser.hadError = 1;
    }
}

void term(int target_reg){
    factor(target_reg);

    while(parser.current.type == TK_PLUS || parser.current.type == TK_MINUS){
        TokenType operator_type = parser.current.type;
        advance();
        int temp_reg = next_reg_free++;
        factor(temp_reg);

        if (operator_type == TK_PLUS){
        emit_instruction(ADD, target_reg, target_reg, temp_reg);
        } else {
            emit_instruction(SUB, target_reg, target_reg, temp_reg);
        }
        next_reg_free--;
    }
}

void expression(int target_reg){
    term(target_reg);

    if(parser.current.type == TK_EQEQ || parser.current.type == TK_LT || parser.current.type == TK_GT){
        TokenType op_type = parser.current.type;
        advance();

        int temp_reg = next_reg_free++;
        term(temp_reg);
            
        if(op_type == TK_EQEQ){
            emit_instruction(EQ, target_reg, target_reg, temp_reg);
        } else if(op_type == TK_LT){
            emit_instruction(LT, target_reg, target_reg, temp_reg);
        } else if(op_type == TK_GT){
            emit_instruction(GT, target_reg, target_reg, temp_reg);
        }
        next_reg_free--;
    }
}