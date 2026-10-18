#include "output.h"
#include "escalonador.h"

static char buffer_saida[10000] = "";

void imprimir_estado_sistema(void) {
    printf("%d\n", sistema.tempo_atual);

    printf("estado NEW");
    TCB *thread = sistema.fila_new.inicio;
    while (thread) {
        printf("\t\tP%02d.T%02d", thread->id_processo, thread->id_thread);
        thread = thread->seguinte;
    }
    printf("\n");

    printf("estado READY");
    thread = sistema.fila_ready.inicio;
    while (thread) {
        printf("\t\tP%02d.T%02d", thread->id_processo, thread->id_thread);
        thread = thread->seguinte;
    }
    printf("\n");

    printf("estado RUN");
    if (sistema.dispatcher_ativo) {
        printf("\t\tDISPATCHER");
    } else if (sistema.thread_em_execucao) {
        printf("\t\tP%02d.T%02d",
               sistema.thread_em_execucao->id_processo,
               sistema.thread_em_execucao->id_thread);
    }
    printf("\n");

    printf("estado BLOCKED");
    thread = sistema.fila_blocked.inicio;
    while (thread) {
        printf("\t\tP%02d.T%02d", thread->id_processo, thread->id_thread);
        thread = thread->seguinte;
    }
    printf("\n");

    printf("estado EXIT");
    thread = sistema.fila_exit.inicio;
    while (thread) {
        printf("\t\tP%02d.T%02d", thread->id_processo, thread->id_thread);
        thread = thread->seguinte;
    }
    printf("\n\n");
}

void imprimir_marcador_tempo(int tempo) {
    printf("%d\n", tempo);
}

void limpar_buffer_saida(void) {
    if (strlen(buffer_saida) > 0) {
        printf("%s\n", buffer_saida);
        buffer_saida[0] = '\0';
    }
}

void registar_instrucao_print(TCB *thread, const char *valor) {
    if (!valor) return;
    char entrada[64];
    if (thread)
        snprintf(entrada, sizeof(entrada), ">> P%02d.T%02d print %s",
                 thread->id_processo, thread->id_thread, valor);
    else
        snprintf(entrada, sizeof(entrada), ">> print %s", valor);

    if (strlen(buffer_saida) > 0)
        strncat(buffer_saida, "\n", sizeof(buffer_saida) - strlen(buffer_saida) - 1);
    strncat(buffer_saida, entrada, sizeof(buffer_saida) - strlen(buffer_saida) - 1);
}