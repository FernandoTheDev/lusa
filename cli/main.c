#include <stdio.h>
#include "../compiler/compiler.h"
#include "../lvm/lvm.h"
#include "lusa_string.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

int ends_with(const char *str, const char *suffix)
{
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    if (len_suffix > len_str) return 0;
    return strcmp(str + len_str - len_suffix, suffix) == 0;
}

void print_help()
{
    printf("Use: lusa <comando> <arquivo>\n\n");
    printf("Comandos:\n\n");
    printf("    run         Compila e executa um script .lusa ou roda um .bc direto.\n");
    printf("    build       Compila um script .lusa e gera o bytecode '.bc'\n\n");
    printf("    man         Abre o manual interativo da linguagem\n\n");
    printf("    version     Mostra a versao da linguagem\n");
    printf("    help        Mostra esta mensagem de ajuda\n");
}

void print_manual()
{
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

// essa função nunca vai ser usada
int check_embedded_and_run(const char *current_exe)
{
    FILE *f;
    if (lusa_fopen(&f, current_exe, "rb") != 0 || !f)
        return 0;
    fseek(f, -8, SEEK_END);

    uint32_t bc_size = 0;
    uint32_t magic = 0;
    fread(&bc_size, sizeof(uint32_t), 1, f);
    fread(&magic, sizeof(uint32_t), 1, f);

    if (magic == LUSA_MAGIC)
    {
        fseek(f, -(8 + bc_size), SEEK_END);

        uint8_t *buffer = malloc(bc_size);
        fread(buffer, 1, bc_size, f);
        fclose(f);

        vm_execute_from_buffer(buffer, bc_size);

        free(buffer);
        return 1;
    }
    fclose(f);
    return 0;
}

// o uso de char** argv
// é mais conselhado por boas praticas
int main(int argc, char **argv)
{

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    // nunca é usado
    // if (check_embedded_and_run(argv[0])){
    //     return 0;
    // }

    if (argc < 2)
    {
        print_help();
        return 1;
    }

    char *command = argv[1];

    // seria mais interessante enviar o comando para uma função externa
    // e ela lidar com todos os comandos e flags possiveis
    // só pra uma organização melhor, aqui o código fica muito denso

    if (strcmp(command, "help") == 0)
    {
        print_help();
    }
    else if (strcmp(command, "man") == 0)
    {
        print_manual();
    }
    else if (strcmp(command, "version") == 0)
    {
        printf("LusaLang v0.0.1\n");
    }
    else if (strcmp(command, "build") == 0)
    {
        if (argc < 3)
        {
            // crie uma macro pro "\003[1;31mERRO:\033[0m "
            // assim:
            // define LUSA_ERROR "\003[1;31mERRO:\033[0m "
            // use assim: printf(LUSA_ERROR "Especifique o arquivo fonte para compilar. Ex: lusa build main.lusa\n");
            printf("\003[1;31mERRO:\033[0m Especifique o arquivo fonte para compilar. Ex: lusa build main.lusa\n");
            return 1;
        }

        char *in_file = argv[2];
        char out_file[256];
        char temp_bc[256];

        snprintf(temp_bc, sizeof(temp_bc), "%s.tmp", in_file);

        if (argc >= 5 && strcmp(argv[3], "-o") == 0)
        {
            snprintf(out_file, sizeof(out_file), "%s", argv[4]);
        }
        else
        {
            lusa_strcpy(out_file, sizeof(out_file), in_file);
            char *dot = strrchr(out_file, '.');
            if (dot)
                lusa_strcpy(dot, 4, ".bc");
        }

        // isso nem faz sentido, voce nao compila pra binario, remove isso
        // ao invés de sempre serializar tudo em um arquivo .txt
        // salva todo o estado em uma struct e passa ela para a VM, vai econimizar IO e vai ser muito melhor

        if (ends_with(out_file, ".exe") || ends_with(out_file, ".elf") || ends_with(out_file, " "))
        {
            if (compile(in_file, temp_bc) == 0)
            {
                if (build_executable(argv[0], temp_bc, out_file) == 0)
                {
                    // use macros aqui também
                    printf("\033[1;32mSucesso!\033[0m Executavel criado: %s\n", out_file);
                }
                else
                {
                    printf("\033[1;31mERRO:\033[0m Falha ao gerar executavel.\n");
                }
                remove(temp_bc);
            }
        }
        else
        {
            compile(in_file, out_file);
        }
    }
    else if (strcmp(command, "run") == 0)
    {
        if (argc < 3)
        {
            printf("ERRO: Especifique o arquivo para executar.\n");
            return 1;
        }

        char *file_to_run = argv[2];
        if (ends_with(file_to_run, ".bc"))
        {
            vm_run(file_to_run);
        }
        else if (ends_with(file_to_run, ".lusa"))
        {
            LusaModule *module = compile_to_module(file_to_run);
            if (module != NULL)
            {
                vm_execute_module(module);
                free_lusa_module(module);
            }
            else
            {
                printf("\033[1;31mFalha na compilacao.\033[0m\n");
            }
        }
        else
        {
            printf("\033[1;31mERRO:\033[0m Extensao de arquivo nao suportada. Use .lusa ou .bc\n");
        }
    }
    else
    {
        printf("ERRO: Comando desconhecido '%s'\n\n", command);
        print_help();
    }

    return 0;
}
