#ifndef ESCALONADOR_H
#define ESCALONADOR_H

#include "main.h"

void inserir_fila(Fila *q, TCB *thread);
TCB* remover_fila(Fila *q);
void remover_thread_da_fila(Fila *q, TCB *thread);
void limpar_fila(Fila *q);
int tamanho_fila(Fila *q);

PCB* criar_processo(void);
TCB* criar_thread(PCB *processo, const char *nome_programa);
TCB* criar_thread_principal(PCB *processo, const char *nome_programa);

void mudar_estado_thread(TCB *thread, EstadoThread novo_estado);
void mover_para_ready(TCB *thread);
void mover_para_blocked(TCB *thread, int tempo_bloqueio);
void mover_para_exit(TCB *thread);

TCB* escalonar(void);
void ativar_dispatcher(void);
void atualizar_threads_bloqueadas(void);
void mover_todas_new_para_ready(void);
int recolher_todas_new_para_ready(TCB *recolhidas[], int tamanho_maximo);
int recolher_threads_bloqueadas_para_ready(TCB *recolhidas[], int tamanho_maximo);
void limpar_threads_exit(void);
void matar_todas_threads_no_processo(PCB *processo);

void destruir_processo(PCB *proc);
void destruir_sistema(void);

#endif