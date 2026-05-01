#include <stdio.h>
#include "parser.h"

#include "../../lvm/lvm.h"
#include "../symtab.h"
#include "../codegen.h"
#include "lusa_string.h"

static inline void var_declaration(){
    consume(TK_ID, "Esperava o nome de variavel apos 'var'.");

    char var_name[50];
    lusa_strcpy(var_name, 50, parser.previus.text);

    int tipo_var = TK_INT;

    if (parser.current.type == TK_INT || parser.current.type == TK_FLOAT || parser.current.type == TK_STRING || parser.current.type == TK_BOOL) {
        tipo_var = parser.current.type;
        advance();
    }

    int reg_id = var_register(var_name, tipo_var);

    
    if (parser.current.type == TK_EQUAL){
        advance();
        if(reg_id != -1){
            int real_type = expression(reg_id);

            for(int i = 0; i < symbol_counter; i++){
                if(strcmp(table[i].name, var_name) == 0){
                    table[i].tipo = real_type;
                    break;
                }
            }
        }
    } else {
        if (reg_id != -1) emit_instruction(LOAD, reg_id, 0, 0);
    }

    consume(TK_SEMICOLON, "Esperava ';' no final da declaracao");
}

void statement();

static void find_while(){
    advance();
    int loop_start = bc_size;
    int cond_reg = next_reg_free++;
    expression(cond_reg);

    int jump_to_exit = bc_size;
    emit_instruction(JMPF, cond_reg, 0, 0);

    next_reg_free--;

    consume(TK_LBRACE, "Esperava '{' apos a condicao do while.");
    while(parser.current.type != TK_RBRACE && parser.current.type != TK_EOF){
        statement();
    }
    consume(TK_RBRACE, "Esperava '}' no final do bloco while.");

    uint8_t rB_start = (loop_start >> 8) & 0xFF;
    uint8_t rC_start = loop_start & 0xFF;
    emit_instruction(JMP, 0, rB_start, rC_start);

    int exit_index = bc_size;
    uint8_t rB_exit = (exit_index >> 8) & 0xFF;
    uint8_t rC_exit = exit_index & 0xFF;

    bytecode[jump_to_exit] = (JMPF << 24) | (cond_reg << 16) | (rB_exit << 8) | rC_exit;
}

static void find_if(){
    advance();

    int cond_reg = next_reg_free++;
    expression(cond_reg);

    int jump_index = bc_size;
    emit_instruction(JMPF, cond_reg, 0, 0);

    next_reg_free--;

    consume(TK_LBRACE, "Esperava '{' apos a condicao do if.");

    while (parser.current.type != TK_RBRACE && parser.current.type != TK_EOF){
        statement();
    }

    consume(TK_RBRACE, "Esperava '}' no final bloco if.");

    if (parser.current.type == TK_ELSE){
        advance();

        int jump_to_end = bc_size;
        emit_instruction(JMP, 0, 0, 0);

        int else_start = bc_size;
        bytecode[jump_index] = (JMPF << 24) | (cond_reg << 16) | ((else_start >> 8) & 0xFF) << 8 | (else_start & 0xFF);

        if (parser.current.type == TK_IF){
            find_if();
        } else {
            consume(TK_LBRACE, "Esperava '{' apos 'else'.");
            while(parser.current.type != TK_RBRACE && parser.current.type != TK_EOF){
                statement();
            }
            consume(TK_RBRACE, "Esperava '}' no final do bloco else.");
        }

        int end_index = bc_size;
        bytecode[jump_to_end] = (JMP << 24) | (0 << 16) | ((end_index >> 8) & 0xFF) << 8 | (end_index & 0xFF);
    } else {
        int end_index = bc_size;
        bytecode[jump_index] = (JMPF << 24) | (cond_reg << 16) | ((end_index >> 8) & 0xFF) << 8 | (end_index & 0xFF);
    }
}

static void assignment() {
    char target_name[50];
    lusa_strcpy(target_name, 50, parser.current.text);

    advance(); 
    
    if (parser.current.type == TK_LPAREN) {
        if (strcmp(target_name, "print") == 0) {
            advance(); 
            //expression(0); // <--- OLHE O ASSASSINO AQUI!
            int temp_reg = next_reg_free++;
            int found_type = expression(temp_reg);
            consume(TK_RPAREN, "Esperava ')' apos print.");
            if(found_type == TK_STRING){
                emit_instruction(CALL_EXT, 0, 2, temp_reg);
            } else if(found_type == TK_FLOAT) {
                emit_instruction(CALL_EXT, 0, 3, temp_reg);
            } else{
            emit_instruction(CALL_EXT, 0, 1, temp_reg);
            }
            next_reg_free--;
            consume(TK_SEMICOLON, "Esperava ';' no final do print.");
            return;
        }
        
        int dest_pc = -1;
        int fn_index = -1;
        for (int i = 0; i < func_counter; i++) {
            if(strcmp(func_table[i].name, target_name) == 0) { 
                dest_pc = func_table[i].start_pc;
                fn_index = i;
            }
        } 

        if (dest_pc == -1) {
            printf("[COMPILADOR] ERRO: funcao '%s' nao existe.\n", target_name);
            parser.hadError = 1;
        } else {
            advance();
            int args_read = 0;
            if (parser.current.type != TK_RPAREN) {
                do {
                    if (parser.current.type == TK_COMMA) advance();

                    if (args_read >= func_table[fn_index].arity) {
                        printf("[COMPILADOR] ERRO: Muitos argumentos para '%s'.\n", target_name);
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
            
            if (args_read < func_table[fn_index].arity) {
                printf("[COMPILADOR] ERRO: funcao '%s' esperava %d argumentos, recebeu %d.\n", target_name, func_table[fn_index].arity, args_read);
                parser.hadError = 1;
            }

            consume(TK_RPAREN, "Esperava ')' apos chamada de funcao.");
            uint8_t rB = (dest_pc >> 8) & 0xFF;
            uint8_t rC = dest_pc & 0xFF;
            emit_instruction(CALL, 0, rB, rC);
        }
        
        consume(TK_SEMICOLON, "Esperava ';' no final da chamada de funcao solta.");
        return; 
    } else if (parser.current.type == TK_LBRACKET){
        advance();
        int index_reg = next_reg_free++;
        expression(index_reg);
        consume(TK_RBRACKET, "Esperava ']' apos o indice");
        
        consume(TK_EQUAL, "Esperava '=' apos o acesso de array");

        int val_reg = next_reg_free++;
        expression(val_reg);
        consume(TK_SEMICOLON, "Esperava ';' no final da atribuicao do array");

        int base_reg = find_var(target_name);
        if (base_reg == -1){
            printf("[COMPILADOR] ERRO: Variavel '%s' nao declarada.\n", target_name);
            parser.hadError = 1;
        } else {
            emit_instruction(STORE, val_reg, base_reg, index_reg);
        }
        
        next_reg_free -= 2;
        return;
    } else if (parser.current.type == TK_EQUAL) {
        
        int reg_id = find_var(target_name);

        if (reg_id == -1) {
            printf("[COMPILADOR] ERRO: Tentando atribuir valor a variavel nao declarada '%s'.\n", target_name);
            parser.hadError = 1;
            return;
        }

        advance(); 
        int value_type = expression(reg_id);
        consume(TK_SEMICOLON, "Esperava ';' no final da atribuicao.");
 
        for(int i = 0; i < symbol_counter; i++){
            if(strcmp(table[i].name, target_name) == 0){
                table[i].tipo = value_type;
                break;
            }
        }
        return;
    } else {
        printf("[COMPILADOR] ERRO de Sintaxe: O que voce quis fazer com '%s'?\n", target_name);
        parser.hadError = 1;
        advance();
    }
}

static void function_declaration(){
    advance();
    consume(TK_ID, "Esperava o nome da funcao.");

    char fn_name[50];
    lusa_strcpy(fn_name, 50, parser.previus.text);

    int jump_over = bc_size;
    emit_instruction(JMP, 0, 0, 0);

    int fn_index = func_counter++;
    lusa_strcpy(func_table[fn_index].name, 50, fn_name);
    func_table[fn_index].start_pc = bc_size;
    func_table[fn_index].arity = 0;

    int saved_symbol_counter = symbol_counter;

    consume(TK_LPAREN, "Esperava '(' apos o nome da funcao.");

    if (parser.current.type != TK_RPAREN){
        do {
            if(parser.current.type == TK_COMMA) advance();

            consume(TK_ID, "Esperava nome do parametro");
            char param_name[50];
            lusa_strcpy(param_name, 50, parser.previus.text);

            int tipo_param = TK_INT;
            if (parser.current.type == TK_INT || parser.current.type == TK_FLOAT){
                tipo_param = parser.current.type;
                advance();
            } else {
                printf("[COMPILADOR] ERRO: Parametro '%s' sem tipo definido.\n", param_name);
                parser.hadError = 1;
            }

            int reg = var_register(param_name, tipo_param);

            func_table[fn_index].param_reg[func_table[fn_index].arity] = reg;
            func_table[fn_index].arity++;
        } while (parser.current.type == TK_COMMA);
    }

    consume(TK_RPAREN, "Esperava ')'");

    consume(TK_LBRACE, "Esperava '{' no inicio da funcao.");
    while(parser.current.type != TK_RBRACE && parser.current.type != TK_EOF){
        statement();
    }
    consume(TK_RBRACE, "Esperava '}' no final da funcao.");

    emit_instruction(RET, 0, 0, 0);

    symbol_counter = saved_symbol_counter;
    int end_index = bc_size;
    uint8_t rB = (end_index >> 8) & 0xFF;
    uint8_t rC = end_index & 0xFF;
    bytecode[jump_over] = (JMP << 24) | (0 << 16) | (rB << 8) | rC;
}

static void return_statement(){
    advance();
    expression(0);
    emit_instruction(RET, 0, 0, 0);
    consume(TK_SEMICOLON, "Esperava ';' apos o return.");
}

void statement(){
    if(parser.current.type == TK_VAR){
        advance();
        var_declaration();
    }else if(parser.current.type == TK_ID){
        assignment();
    }else if(parser.current.type == TK_FN){
        function_declaration();
    }else if(parser.current.type == TK_RETURN){
        return_statement();
    }else if(parser.current.type == TK_IF){
        find_if();
    }else if(parser.current.type == TK_WHILE){
        find_while();
    }else {
        printf("[COMPILADOR] ERRO: token inesperado '%s'\n", parser.current.text);
        advance();
    }
}