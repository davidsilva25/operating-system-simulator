#include "executor.h"
#include "escalonador.h"
#include "memoria.h"
#include "output.h"
#include "inputs.h"

static TCB* thread_criada_sistema = NULL;

static PCB* encontrar_processo(int pid) {
    PCB *atual = sistema.processos;
    while (atual && atual->id_processo != pid) atual = atual->seguinte;
    return atual;
}

void executar_instrucao(TCB *thread, Instrucao instr) {
    if (!thread || !thread->programa) return;

    PCB *processo = encontrar_processo(thread->id_processo);
    if (!processo) return;

    switch (instr.tipo) {
        case WORK:
            break;

        case BLOCK: {
            int tempo_bloqueio = instr.arg1;
            sistema.thread_em_execucao = NULL;
            mover_para_blocked(thread, tempo_bloqueio);
            break;
        }

        case SET: {
            if (instr.arg1 < 10)
                definir_var_global(processo, instr.arg1, instr.arg2);
            else
                definir_var_local(thread, instr.arg1 - 10, instr.arg2);
            break;
        }

        case ADD: {
            if (instr.arg1 < 10) {
                int val = obter_var_global(processo, instr.arg1);
                definir_var_global(processo, instr.arg1, val + instr.arg2);
            } else {
                int val = obter_var_local(thread, instr.arg1 - 10);
                definir_var_local(thread, instr.arg1 - 10, val + instr.arg2);
            }
            break;
        }

        case PRINT: {
            char buffer[32];
            if (instr.arg1 < 10)
                sprintf(buffer, "%d", obter_var_global(processo, instr.arg1));
            else
                sprintf(buffer, "%d", obter_var_local(thread, instr.arg1 - 10));
            registar_instrucao_print(thread, buffer);
            break;
        }

        case JUMP: {
            thread->contador_programa += instr.arg2 - 1; 
            break;
        }

        case JZ: {
            int valor;
            if (instr.arg1 < 10)
                valor = obter_var_global(processo, instr.arg1);
            else
                valor = obter_var_local(thread, instr.arg1 - 10);

            if (valor == 0)
                thread->contador_programa += instr.arg2 - 1;
            break;
        }

        case PROC_CREATE: {
            PCB *novo_proc = criar_processo();
            if (novo_proc) {
                TCB *nova_thread = criar_thread_principal(novo_proc, instr.arg_str);
                if (nova_thread) {
                    inserir_fila(&sistema.fila_new, nova_thread);
                } else {
                    registar_instrucao_print(thread, "falha de proc_create");
                }
            } else {
                registar_instrucao_print(thread, "falha de proc_create");
            }
            break;
        }

        case THREAD_CREATE: {
            TCB *nova_thread = criar_thread(processo, instr.arg_str);
            if (nova_thread) {
                mudar_estado_thread(nova_thread, READY);
                thread_criada_sistema = nova_thread;
            } else {
                registar_instrucao_print(thread, "falha de thread_create");
            }
            break;
        }

        case END: {
            sistema.thread_em_execucao = NULL;
            if (thread->id_thread == 0) {
                mover_para_exit(thread);
                matar_todas_threads_no_processo(processo);
            } else {
                if (processo->threads == thread) {
                    processo->threads = thread->seguinte_proc;
                } else {
                    TCB *t = processo->threads;
                    while (t && t->seguinte_proc != thread) t = t->seguinte_proc;
                    if (t) t->seguinte_proc = thread->seguinte_proc;
                }
                processo->total_threads--;
                if (thread->numero_pagina >= 0) {
                    libertar_pagina(thread->numero_pagina);
                }
                free(thread);
            }
            break;
        }
    }

    if (instr.tipo != END) {
        thread->contador_programa++;
    }
}

void executar_ciclo_cpu(void) {
    limpar_threads_exit();

    TCB *threads_criadas[100];
    int total_criadas = 0;
    
    TCB *threads_bloqueadas_para_ready[100];
    int total_bloqueadas = 0;
    
    TCB *thread_run_para_ready[1];
    int total_run = 0;
    
    TCB *threads_new_para_ready[100];
    int total_new = 0;

    total_new = recolher_todas_new_para_ready(threads_new_para_ready, 100);

    total_bloqueadas = recolher_threads_bloqueadas_para_ready(threads_bloqueadas_para_ready, 100);

    thread_criada_sistema = NULL;

    bool dispatcher_estava_ativo = sistema.dispatcher_ativo;

    if (sistema.dispatcher_ativo) {
        sistema.dispatcher_ativo = false;
        TCB *proxima_thread = escalonar();
        if (proxima_thread) {
            sistema.thread_em_execucao = proxima_thread;
            mudar_estado_thread(proxima_thread, RUN);
        }
    } else {
        TCB *atual = sistema.thread_em_execucao;
        if (atual && atual->programa) {
            
            if (atual->contador_programa >= atual->programa->total_instrucoes || atual->contador_programa < 0) {
                sistema.thread_em_execucao = NULL;
                
                if (atual->contador_programa > atual->programa->total_instrucoes || atual->contador_programa < 0) {
                    registar_instrucao_print(atual, "segmentation fault");
                }
                
                if (atual->id_thread == 0) {
                    matar_todas_threads_no_processo(encontrar_processo(atual->id_processo));
                    mover_para_exit(atual);
                } else {
                    PCB *processo = encontrar_processo(atual->id_processo);
                    if (processo) {
                        if (processo->threads == atual) {
                            processo->threads = atual->seguinte;
                        } else {
                            TCB *t = processo->threads;
                            while (t && t->seguinte != atual) t = t->seguinte;
                            if (t) t->seguinte = atual->seguinte;
                        }
                        processo->total_threads--;
                    }
                    if (atual->numero_pagina >= 0) {
                        libertar_pagina(atual->numero_pagina);
                    }
                    free(atual);
                }
            } else {
                int id_pagina = atual->contador_programa / TAMANHO_PAGINA;
                int offset = atual->contador_programa % TAMANHO_PAGINA;
                int instr_empacotada = ler_memoria(atual->programa->numeros_pagina[id_pagina], offset);

                Instrucao instr = desempacotar_instrucao(instr_empacotada);
                executar_instrucao(atual, instr);

                if (sistema.thread_em_execucao == atual) {
                    atual->tempo_cpu_usado++;
                    if (atual->tempo_cpu_usado >= QUANTUM) {
                        mudar_estado_thread(atual, READY);
                        atual->tempo_cpu_usado = 0;
                        thread_run_para_ready[total_run++] = atual;
                        sistema.thread_em_execucao = NULL;
                        sistema.dispatcher_ativo = true;
                    }
                }
            }
        }
    }

    if (thread_criada_sistema != NULL) {
        threads_criadas[total_criadas++] = thread_criada_sistema;
    }

    for (int i = 0; i < total_criadas; i++) {
        inserir_fila(&sistema.fila_ready, threads_criadas[i]);
    }
    for (int i = 0; i < total_bloqueadas; i++) {
        inserir_fila(&sistema.fila_ready, threads_bloqueadas_para_ready[i]);
    }
    for (int i = 0; i < total_run; i++) {
        inserir_fila(&sistema.fila_ready, thread_run_para_ready[i]);
    }
    for (int i = 0; i < total_new; i++) {
        inserir_fila(&sistema.fila_ready, threads_new_para_ready[i]);
    }

    if (!dispatcher_estava_ativo && !sistema.thread_em_execucao && sistema.fila_ready.inicio != NULL) {
        sistema.dispatcher_ativo = true;
    }
}