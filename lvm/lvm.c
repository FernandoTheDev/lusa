#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "lvm.h"
#include "lusa_utils.h"

#define GET_OPCODE(inst) ((inst >> 24) & 0xFF)
#define GET_REGA(inst) ((inst >> 16) & 0xFF)
#define GET_REGB(inst) ((inst >> 8) & 0xFF)
#define GET_REGC(inst) (inst & 0xFF)
 
int vm_run(const char* filepath){
    
    uint32_t program[1024] = {0};
    int64_t reg[256] = {0};
    int64_t pc = 0;

    uint64_t call_stack[256] = {0};
    int sp = 0;

    char strings[100][100];

    int heap_capacity = 4096 * 4096;
    int64_t* heap = (int64_t*)calloc(heap_capacity, sizeof(int64_t));
    int heap_pointer = 0;
    if (heap == NULL){
        printf("[VM] ERRO FATAL: Falha ao alocar memoria para o Heap.\n");
        return -1;
    }

    FILE* file;
    if (lusa_fopen(&file, filepath, "rb") != 0 || file == NULL){
        printf("[LVM] ERRO: Nao foi possivel abrir o arquivo '%s'\n", filepath);
        return -1;
    }

    // 1. Lê a quantidade de bytecodes
    int lido_bc_size = 0;
    fread(&lido_bc_size, sizeof(int), 1, file);
    
    // 2. Lê os bytecodes e joga no array 'program'
    fread(program, sizeof(uint32_t), lido_bc_size, file);

    // 3. Lê a quantidade de textos
    int read_str_count = 0;
    fread(&read_str_count, sizeof(int), 1, file);

    // 4. Se tiver textos, lê e joga na matriz 'strings'
    if (read_str_count > 0) {
        fread(strings, sizeof(char) * 100, read_str_count, file);
    }

    fclose(file);

    int isRunning = 1;
    while(isRunning){
        uint32_t instruction = program[pc];
        uint8_t op = GET_OPCODE(instruction);
        uint8_t rA = GET_REGA(instruction);
        uint8_t rB = GET_REGB(instruction);
        uint8_t rC = GET_REGC(instruction);

        switch(op){
            case HALT:
                isRunning = 0;
                break;
            case LOAD:
                uint16_t valor = (rB << 8) | rC;
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
                uint16_t endereco = (rB << 8) | rC;
                call_stack[sp++] = pc;
                pc = endereco - 1;
                break;
            }
            case CALL_EXT:{
                if (rB == 1){
                    printf("%" PRId64 "\n", reg[rC]);
                } else if (rB == 2) {
                    printf("%s\n", (char*)reg[rC]);
                } else {
                    return -1;
                }
                break;
            }
            case LOAD_STR:{
                uint16_t str_idx = (rB << 8) | rC;
                reg[rA] = (uint64_t)(strings[str_idx]);
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
                int64_t tamanho = reg[rB];

                if(heap_pointer + tamanho >= heap_capacity){
                    printf("[VM] ERRO FATAL: Out of Memory!\n");
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
                int64_t endereco_real = base_ptr + index;

                if (endereco_real < 0 || endereco_real >= heap_capacity){
                    printf("[VM] ERRO FATAL: Segmentation Fault! (%" PRId64 ")\n", endereco_real);
                    isRunning = 0;
                    break;
                }

                heap[endereco_real] = reg[rA];
                break;
            }
            case READ:{
                int64_t base_ptr = reg[rB];
                int64_t index = reg[rC];
                int64_t endereco_real = base_ptr + index;

                if(endereco_real < 0 || endereco_real >= heap_capacity){
                    printf("[VM] ERRO FATAL: Segmentation Fault %" PRId64 "!\n", endereco_real);
                    isRunning = 0;
                    break;
                }

                reg[rA] = heap[endereco_real];
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
