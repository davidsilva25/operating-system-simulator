#include "main.h"
#include "escalonador.h"
#include "executor.h"
#include "memoria.h"
#include "inputs.h"
#include "output.h"

int carregar_programa_de_ficheiro(const char *nome_ficheiro) {
    if (sistema.total_programas >= 50) return 0;
    return carregar_programa(nome_ficheiro, &sistema.programas[sistema.total_programas++]);
}

void inicializar_sistema(void) {
    memset(&sistema, 0, sizeof(Sistema));
    inicializar_memoria();
}

int main(int argc, char *argv[]) {
    inicializar_sistema();

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <ficheiro_entrada>\n", argv[0]);
        fprintf(stderr, "O ficheiro deve conter:\n");
        fprintf(stderr, "  LOAD programa.prog\n");
        fprintf(stderr, "  START programa\n");
        return 1;
    }

    FILE *entrada = fopen(argv[1], "r");
    if (!entrada) {
        fprintf(stderr, "Erro ao abrir ficheiro de entrada: %s\n", argv[1]);
        return 1;
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), entrada)) {
        size_t len = strlen(linha);
        if (len > 0 && linha[len - 1] == '\n') linha[len - 1] = '\0';

        if (strncmp(linha, "LOAD", 4) == 0) {
            char nome_ficheiro[128];
            sscanf(linha, "LOAD %127s", nome_ficheiro);
            char caminho_completo[256];
            snprintf(caminho_completo, sizeof(caminho_completo), "programs/%s", nome_ficheiro);
            carregar_programa_de_ficheiro(caminho_completo);

        } else if (strncmp(linha, "START", 5) == 0) {
            char nome_prog[32];
            sscanf(linha, "START %31s", nome_prog);

            PCB *processo_inicial = criar_processo();
            if (processo_inicial) {
                TCB *thread_inicial = criar_thread_principal(processo_inicial, nome_prog);
                if (thread_inicial) {
                    thread_inicial->tempo_chegada_new = -1;
                    inserir_fila(&sistema.fila_new, thread_inicial);
                }
            }
        }
    }
    fclose(entrada);

    for (sistema.tempo_atual = 0; sistema.tempo_atual < MAX_UNIDADES_TEMPO; sistema.tempo_atual++) {
        imprimir_estado_sistema();
        executar_ciclo_cpu();
        limpar_buffer_saida();
    }

    destruir_sistema();
    return 0;
}