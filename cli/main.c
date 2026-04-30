#include <stdio.h>
#include "../compiler/compiler.h"
#include "../lvm/lvm.h"

#define LUSA_MAGIC 0x4C555341

int ends_with(const char *str, const char* suffix){
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    if(len_suffix > len_str) return 0;
    return strcmp(str + len_str - len_suffix, suffix) == 0;
}

void print_help(){
    printf("Use: lusa <comando> <arquivo>\n\n");
    printf("Comandos:\n\n");
    printf("    run         Compila e executa um script .lusa ou roda um .bc direto.\n");
    printf("    build       Compila um script .lusa e gera o bytecode '.bc'\n\n");
    printf("    man         Abre o manual interativo da linguagem\n\n");
    printf("    version     Mostra a versao da linguagem\n");
    printf("    help        Mostra esta mensagem de ajuda\n");
}

void print_manual() {
    printf("===================================================\n");
    printf("               MANUAL DA LUSALANG\n");
    printf("===================================================\n\n");
    printf("1. VARIAVEIS:\n");
    printf("   var idade = 20;\n");
    printf("   var nome string = \"Lusa\";\n");
    printf("   var ativo bool = true;\n\n");
    printf("2. ARRAYS:\n");
    printf("   var notas = [10, 8, 9];\n");
    printf("   print(notas[0]); // Imprime 10\n");
    printf("   notas[1] = 99;\n\n");
    printf("3. CONTROLE DE FLUXO:\n");
    printf("   if (idade > 18) {\n");
    printf("       print_str(\"Maior de idade\");\n");
    printf("   } else {\n");
    printf("       print_str(\"Menor de idade\");\n");
    printf("   }\n\n");
    printf("4. LACOS DE REPETICAO:\n");
    printf("   var i = 0;\n");
    printf("   while (i < 5) {\n");
    printf("       print(i);\n");
    printf("       i = i + 1;\n");
    printf("   }\n\n");
    printf("===================================================\n");
}

int build_executable(const char* current_exe, const char* bc_file, const char* out_exe){
    FILE* f_exe;
    if(fopen_s(&f_exe, current_exe, "rb") != 0 || !f_exe){
        printf("ERRO: Nao foi possivel ler o compilador base '%s'.\n", current_exe);
        return -1;
    }

    FILE* f_bc;
    if (fopen_s(&f_bc, bc_file, "rb") != 0 || !f_bc){
        fclose(f_exe);
        return -1;
    }

    FILE* f_out;
    if (fopen_s(&f_out, out_exe, "wb") != 0 || !f_out){
        fclose(f_exe); fclose(f_bc);
        return -1;
    }

    char buffer[4096];
    size_t bytes;
    while((bytes = fread(buffer, 1, sizeof(buffer), f_exe)) > 0){
        fwrite(buffer, 1, bytes, f_out);
    }

    uint32_t bc_size = 0;
    while((bytes = fread(buffer, 1, sizeof(buffer), f_bc)) > 0){
        fwrite(buffer, 1, bytes, f_out);
        bc_size += (uint32_t)bytes;
    }

    uint32_t magic = LUSA_MAGIC;
    fwrite(&bc_size, sizeof(uint32_t), 1, f_out);
    fwrite(&magic, sizeof(uint32_t), 1, f_out);

    fclose(f_exe); fclose(f_bc); fclose(f_out);
    return 0;
}

int check_embedded_and_run(const char* current_exe){
    FILE* f;
    if(fopen_s(&f, current_exe, "rb") != 0 || !f) return 0;
    fseek(f, -8, SEEK_END);

    uint32_t bc_size = 0;
    uint32_t magic = 0;
    fread(&bc_size, sizeof(uint32_t), 1, f);
    fread(&magic, sizeof(uint32_t), 1, f);

    if(magic == LUSA_MAGIC){
        fseek(f, -(8 + bc_size), SEEK_END);

        FILE* f_tmp;
        if(fopen_s(&f_tmp, "embedded_temp.bc", "wb") == 0 && f_tmp){
            char buffer[4096];
            uint32_t left = bc_size;
            while (left > 0){
                size_t to_read = left < sizeof(buffer) ? left : sizeof(buffer);
                size_t bytes = fread(buffer, 1, to_read, f);
                fwrite(buffer, 1, bytes, f_tmp);
                left -= (uint32_t)bytes;
            }
            fclose(f_tmp);
        }
        fclose(f);
        
        vm_run("embedded_temp.bc");
        remove("embedded_temp.bc");
        return 1;
    }
    fclose(f);
    return 0;
}

int main(int argc, char* argv[]){

    if (check_embedded_and_run(argv[0])){
        return 0;
    }

    if (argc < 2){
        print_help();
        return 1;
    }

    char* command = argv[1];
    
    if(strcmp(command, "help") == 0){
        print_help();
    } else if(strcmp(command, "man") == 0){
        print_manual();
    } else if(strcmp(command, "version") == 0){
        printf("LusaLang v0.0.1\n");
    } else if (strcmp(command, "build") == 0){
        if(argc < 3){
            printf("ERRO: Especifique o arquivo fonte para compilar. Ex: lusa build main.lusa\n");
            return 1;
        }

        char* in_file = argv[2];
        char out_file[100];

        if (argc >= 5 && strcmp(argv[3], "-o") == 0){
            snprintf(out_file, sizeof(out_file), "%s", argv[4]);
        } else {
            snprintf(out_file, sizeof(out_file), "out.bc");
        }

        if (ends_with(out_file, ".exe")){
            if(compile(in_file, "temp.bc") == 0){
                if(build_executable(argv[0], "temp.bc", out_file) == 0){
                    printf("Sucesso! Executavel criado.\n");
                } else {
                    printf("ERRO fatal ao criar executavel.\n");
                }

                remove("temp.bc");
            }
        } else {
            compile(in_file, out_file);
        }
        
    } else if (strcmp(command, "run") == 0){
        if(argc < 3){
            printf("ERRO: Especifique o arquivo para executar.\n");
            return 1;
        }

        char* file_to_run = argv[2];
        if(ends_with(file_to_run, ".bc")){
            vm_run(file_to_run);
        }

        else if(ends_with(file_to_run, ".lusa")){
            if(compile(file_to_run, "temp.bc") == 0){
                vm_run("temp.bc");
                remove("temp.bc");
            } else {
                printf("Falha na compilacao. Execucao abortada.\n");
            }
        }
        else{
            printf("ERRO: Extensao de arquivo nao suportada. Use .lusa ou .bc\n");
        }
    }

    else{
        printf("ERRO: Comando desconhecido '%s'\n\n", command);
        print_help();
    }

    return 0;
}