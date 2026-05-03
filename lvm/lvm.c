#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "lusa_string.h"
#include "lvm.h"
#include "lusa_utils.h"

#define GET_OPCODE(inst) ((inst >> 24) & 0xFF)
#define GET_REGA(inst) ((inst >> 16) & 0xFF)
#define GET_REGB(inst) ((inst >> 8) & 0xFF)
#define GET_REGC(inst) (inst & 0xFF)

void free_lusa_module(LusaModule* module) {
    if (module == NULL) return;
    if (module->code) free(module->code);
    if (module->strings) free(module->strings);
    if (module->floats) free(module->floats);
    free(module);
}


int vm_execute_module(LusaModule* module){
    if(module == NULL || module->code == NULL){
        printf("[LVM] \033[1;31mERRO FATAL:\033[0m Tentativa de executar modulo nulo.\n");
        return -1;
    }

    int64_t reg[256] = {0};
    int64_t pc = 0;
    uint64_t call_stack[256] = {0};
    size_t sp = 0;

    uint64_t heap_capacity = 8196 * 8196;
    int64_t* heap = (int64_t*)calloc(heap_capacity, sizeof(int64_t));
    int heap_pointer = 0;

    if (heap == NULL){
        printf("[VM] \033[1;31mERRO FATAL:\033[0m Falha ao alocar memoria para o Heap.\n");
        return -1;
    }

    int isRunning = 1;
    while(isRunning){
        if(pc >= module->code_size){
            printf("[LVM] \033[1;31mERRO:\033[0m contador fora dos limites do programa.\n");
            break;
        }

        uint32_t instruction = module->code[pc];
        uint8_t op = GET_OPCODE(instruction);
        uint8_t rA = GET_REGA(instruction);
        uint8_t rB = GET_REGB(instruction);
        uint8_t rC = GET_REGC(instruction);

        switch(op){
            case HALT:
                isRunning = 0;
                break;
            case LOAD:
                uint16_t valor = (uint16_t)((rB << 8) | rC);
                reg[rA] = valor;
                break;
            case MOV:
                reg[rA] = reg[rB];
                break;
            case ADD:{
                reg[rA] = reg[rB] + reg[rC];
                break;
            }
            case FADD:{
                double valB = *((double*)&reg[rB]);
                double valC = *((double*)&reg[rC]);
                double res = valB + valC;
                reg[rA] = *((int64_t*)&res);
                break;
            }
            case SUB:{
                reg[rA] = reg[rB] - reg[rC];
                break;
            }
            case FSUB:{
                double valB = *((double*)&reg[rB]);
                double valC = *((double*)&reg[rC]);
                double res = valB - valC;
                reg[rA] = *((int64_t*)&res);
                break;
            }
            case EQ:{
                reg[rA] = (reg[rB] == reg[rC]) ? 1 : 0;
                break;
            }
            case CMP:{
                int64_t idx1 = reg[rB];
                int64_t idx2 = reg[rC];
                if(idx1 < 0 || idx1 >= module->string_count || idx2 < 0 || idx2 >= module->string_count){
                    printf("[LVM] \033[1;31m ERRO FATAL:\033[1;0m Indice de string invalido na comparacao");
                    isRunning = 0;
                    break;
                }
                if (strcmp(module->strings[idx1], module->strings[idx2]) == 0){
                    reg[rA] = 1;
                } else {
                    reg[rA] = 0;
                }
                break;
            }
            case LT: {
                reg[rA] = (reg[rB] < reg[rC]) ? 1 : 0;
                break;
            }
            case GT: {
                reg[rA] = (reg[rB] > reg[rC]) ? 1 : 0;
                break;
            }
            case JMPF: {
                if (reg[rA] == 0){
                    uint16_t endereco = (rB << 8) | rC;
                    pc = endereco - 1;
                } else {
                }
                break;
            }
            case JMP: {
                uint16_t endereco = (rB << 8) | rC;
                pc = endereco - 1;
                break;
            }
            case CALL: {
                if (sp >= sizeof(call_stack) / sizeof(call_stack[0])){
                    printf("[LVM] \033[1;31mERRO FATAL:\033[1;0m Call Stack Overflow!\n");
                    isRunning = 0;
                    break;
                }
                uint16_t endereco = (rB << 8) | rC;
                call_stack[sp++] = pc;
                pc = endereco - 1;
                break;
            }
            case CALL_EXT:{
                if (rB == 1){
                    printf("%" PRId64 "\n", reg[rC]);
                } else if (rB == 2) {
                    int64_t val = reg[rC];
                    if (val < module->string_count){
                        if (val < 0 || val >= module->string_count){
                            printf("[VM] \033[1;31mERRO FATAL:\033[1;0m Tentativa de imprimir string inexistente (Index: %" PRId64 ")\n", val);
                            isRunning = 0;
                            break;
                        }
                        printf("%s\n", module->strings[val]);
                    } else {
                    uint64_t heap_idx = val - module->string_count;
                    if (heap_idx < 0 || heap_idx >= heap_capacity){
                        printf("[VM] \033[1;31mERRO:\033[0m Acesso inválido ao Heap no endereço: %" PRId64 "\n", val);
                            isRunning = 0;
                        } else {
                            printf("%s\n", (char*)&heap[heap_idx]);
                        }
                    }
                } else if (rB == 3){
                    double val = *((double*)&reg[rC]);
                    printf("%f\n", val);
                }
                break;
            }
            case LOAD_STR:{
                reg[rA] = (rB << 8) | rC;
                break;
            }
            case LOAD_FLT:{
                uint16_t idx = (rB << 8) | rC;
                if (module->floats == NULL || idx >= module->float_count){
                    printf("[LVM] \033[1;31mERRO FATAL:\033[1;0m Tentativa de leitura fora dos limites (Float Pool Index: %d)\n", idx);
                    isRunning = 0;
                    break;
                }
                double value = module->floats[idx];
                reg[rA] = *((int64_t*)&value);
                break;
            }
            case RET: {
                if (sp > 0){
                    pc = call_stack[--sp];
                } else{
                    isRunning = 0;
                }
                break;
            }
            case ALLOC:{
                uint64_t tamanho = reg[rB];

                if(tamanho <= 0 || heap_pointer + tamanho >= heap_capacity){
                    printf("[VM]\033[1;31mPANIC:\033[1;0m Out of Memory!\n");
                    isRunning = 0;
                    break;
                }

                reg[rA] = heap_pointer;
                heap_pointer += tamanho;
                break;
            }
            case STORE:{
                int64_t base_ptr = reg[rB];
                int64_t index = reg[rC];
                uint64_t endereco_real = base_ptr + index;

                if (index < 0 || endereco_real < 0 || endereco_real >= heap_capacity){
                    printf("[VM] \033[1;31mPANIC:\033[1;0m Segmentation Fault! (%" PRId64 ")\n", endereco_real);
                    isRunning = 0;
                    break;
                }

                heap[endereco_real] = reg[rA];
                break;
            }
            case READ:{
                int64_t base_ptr = reg[rB];
                int64_t index = reg[rC];
                uint64_t endereco_real = base_ptr + index;

                if(index < 0 || endereco_real < 0 || endereco_real >= heap_capacity){
                    printf("[VM] \033[1;31mPANIC:\033[1;0m Segmentation Fault %" PRId64 "!\n", endereco_real);
                    isRunning = 0;
                    break;
                }

                reg[rA] = heap[endereco_real];
                break;
            }
            case SCAT:{
                int64_t idx1 = reg[rB];
                int64_t idx2 = reg[rC];
                if(idx1 < 0 || idx1 >= module->string_count || idx2 < 0 || idx2 >= module->string_count){
                    printf("[LVM] \033[1;31mERRO FATAL:\033[0m Índice de string inválido na concatenação.\n");
                    isRunning = 0;
                    break;
                }
                const char* s1 = module->strings[idx1];
                const char* s2 = module->strings[idx2];
                size_t new_size = strlen(s1) + strlen (s2) + 1;
                uint64_t elements = (new_size + 7) / 8;

                if (heap_pointer + elements >= heap_capacity){
                    printf("[VM]\033[1;31mPANIC:\033[0m Heap cheio para concatenação!\n");
                    isRunning = 0;
                    break;
                }

                reg[rA] = heap_pointer + module->string_count;

                char* destino = (char*)&heap[heap_pointer];
                lusa_strcpy(destino, new_size, s1);
                lusa_strcat(destino, new_size, s2);

                heap_pointer += elements;
                break;
            }
            default:
                printf("ERRO: Instrucao desconhecida (%d) no PC: %" PRId64 "\n", op, pc);
                isRunning = 0;
                break;
        }   

        pc++;
    }
    free(heap);
    return 0;
}


// remove isso, é terrivel, mantenha tudo em memoria, a VM nunca deve ter acesso ou noção disso
int vm_run(const char* filepath){
    FILE* file;
    if(lusa_fopen(&file, filepath, "rb") != 0) {
        printf("\033[1;31mERRO FATAL:\033[0m nao foi possivel iniciar a VM.");
        return -1;
    }

    LusaModule* module = malloc(sizeof(LusaModule));

    fread(&module->code_size, sizeof(int), 1, file);
    if (module->code_size <= 0 || module->code_size > 1000000) {
        printf("[LVM] ERRO: Tamanho de código inválido ou suspeito.\n");
        fclose(file); return -1;
    } else {
        module->code = malloc(sizeof(uint32_t) * module->code_size);
        fread(module->code, sizeof(uint32_t), module->code_size, file);
    }
    
    fread(&module->string_count, sizeof(int), 1, file);
    if (module->string_count < 0 || module->string_count > 5000) {
        printf("[LVM] ERRO: Tabela de strings excede os limites de segurança.\n");
        fclose(file); return -1;
    } else {
        module->strings = malloc(sizeof(char[1000]) * module->string_count);
        if (!module->strings) {
            printf("[LVM] ERRO: Falha crítica de memória (Out of Memory).\n");
            free(module);
            fclose(file); return -1;
        } else {
            fread(module->strings, sizeof(char[1000]), module->string_count, file);
        }
    }

    fread(&module->float_count, sizeof(int), 1, file);
    if (module->float_count > 0){
        module->floats = malloc(sizeof(double) * module->float_count);
        fread(module->floats, sizeof(double), module->float_count, file);
    } else module->floats = NULL;

    fclose(file);

    int result = vm_execute_module(module);
    free_lusa_module(module);
    return result;
}

// lvm/lvm.c
void vm_execute_from_buffer(unsigned char* buffer, uint32_t size) {
    if(buffer == NULL || size == 0){
        printf("[LVM] \033[1;31mERRO:\033[0m Buffer de bytecode inválido.\n");
        return;
    }

    LusaModule module = {0};
    uint32_t offset = 0;

    module.code_size = *(int*)(buffer + offset);
    offset += sizeof(int);
    module.code = (uint32_t*)(buffer + offset);
    offset += sizeof(uint32_t) * module.code_size;
    
    module.string_count = *(int*)(buffer + offset);
    offset += sizeof(int);
    module.strings = (char (*)[1000])(buffer + offset);
    offset += sizeof(char[1000]) * module.string_count;

    module.float_count = *(int*)(buffer + offset);
    offset += sizeof(int);
    if (module.float_count > 0) {
        module.floats = (double*)(buffer + offset);
    } else {
        module.floats = NULL;
    }

    vm_execute_module(&module);
    
}
