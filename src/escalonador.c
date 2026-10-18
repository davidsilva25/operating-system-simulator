#include "escalonador.h"
#include "memoria.h"
#include "inputs.h"

Sistema sistema = {0};

void inserir_fila(Fila *q, TCB *thread) {
    if (!q || !thread) return;
    
    thread->seguinte = NULL;
    if (q->fim == NULL) {
        q->inicio = q->fim = thread;
    } else {
        q->fim->seguinte = thread;
        q->fim = thread;
    }
}

TCB* remover_fila(Fila *q) {
    if (!q || q->inicio == NULL) return NULL;
    
    TCB *thread = q->inicio;
    q->inicio = thread->seguinte;
    if (q->inicio == NULL) {
        q->fim = NULL;
    }
    thread->seguinte = NULL;
    return thread;
}

void remover_thread_da_fila(Fila *q, TCB *thread) {
    if (!q || !thread) return;
    
    if (q->inicio == thread) {
        q->inicio = thread->seguinte;
        if (q->inicio == NULL) q->fim = NULL;
    } else {
        TCB *atual = q->inicio;
        while (atual && atual->seguinte != thread) {
            atual = atual->seguinte;
        }
        if (atual) {
            atual->seguinte = thread->seguinte;
            if (thread == q->fim) {
                q->fim = atual;
            }
        }
    }
    thread->seguinte = NULL;
}

void limpar_fila(Fila *q) {
    if (!q) return;
    while (q->inicio) {
        q->inicio = q->inicio->seguinte;
    }
    q->fim = NULL;
}

int tamanho_fila(Fila *q) {
    if (!q) return 0;
    int contador = 0;
    TCB *atual = q->inicio;
    while (atual) {
        contador++;
        atual = atual->seguinte;
    }
    return contador;
}

PCB* criar_processo(void) {
    if (sistema.total_processos >= MAX_PROCESSOS) {
        return NULL;
    }
    
    PCB *proc = (PCB*)malloc(sizeof(PCB));
    if (!proc) return NULL;
    
    proc->id_processo = sistema.total_processos++;
    memset(&proc->vars_globais, 0, sizeof(VariaveisGlobais));
    proc->threads = NULL;
    proc->total_threads = 0;
    proc->numero_pagina = alocar_pagina();
    
    proc->seguinte = sistema.processos;
    sistema.processos = proc;
    
    return proc;
}

TCB* criar_thread(PCB *processo, const char *nome_programa) {
    if (!processo) return NULL;
    
    TCB *thread = (TCB*)malloc(sizeof(TCB));
    if (!thread) return NULL;
    
    thread->id_processo = processo->id_processo;
    thread->id_thread  = processo->total_threads++;
    thread->estado = NEW;
    thread->contador_programa = 0;
    thread->tempo_cpu_usado = 0;
    thread->tempo_bloqueado = 0;
    thread->tempo_chegada_new = sistema.tempo_atual;
    memset(&thread->vars_locais, 0, sizeof(VariaveisLocais));
    thread->seguinte = NULL;
    
    thread->numero_pagina = alocar_pagina();
    
    thread->programa = obter_programa(nome_programa);
    if (!thread->programa) {
        free(thread);
        return NULL;
    }
    
    thread->seguinte_proc = NULL;

    TCB *t = processo->threads;
    if (!t) {
        processo->threads = thread;
    } else {
        while (t->seguinte_proc) t = t->seguinte_proc;
        t->seguinte_proc = thread;
    }
    
    return thread;
}

TCB* criar_thread_principal(PCB *processo, const char *nome_programa) {
    if (!processo) return NULL;
    
    TCB *thread = (TCB*)malloc(sizeof(TCB));
    if (!thread) return NULL;
    
    thread->id_processo = processo->id_processo;
    thread->id_thread  = 0;
    thread->estado = NEW;
    thread->contador_programa = 0;
    thread->tempo_cpu_usado = 0;
    thread->tempo_bloqueado = 0;
    thread->tempo_chegada_new = sistema.tempo_atual;
    memset(&thread->vars_locais, 0, sizeof(VariaveisLocais));
    thread->seguinte = NULL;
    thread->seguinte_proc = NULL;
    
    thread->numero_pagina = alocar_pagina();
    
    thread->programa = obter_programa(nome_programa);
    if (!thread->programa) {
        free(thread);
        return NULL;
    }
    
    processo->threads = thread;
    processo->total_threads = 1;
    
    return thread;
}

void mudar_estado_thread(TCB *thread, EstadoThread novo_estado) {
    if (!thread) return;
    thread->estado = novo_estado;
}

void mover_para_ready(TCB *thread) {
    if (!thread) return;
    mudar_estado_thread(thread, READY);
    thread->tempo_cpu_usado = 0;
    inserir_fila(&sistema.fila_ready, thread);
}

void mover_para_blocked(TCB *thread, int tempo_bloqueio) {
    if (!thread) return;
    mudar_estado_thread(thread, BLOCKED);
    thread->tempo_bloqueado = tempo_bloqueio;
    inserir_fila(&sistema.fila_blocked, thread);
}

void mover_para_exit(TCB *thread) {
    if (!thread) return;
    mudar_estado_thread(thread, EXIT);
    inserir_fila(&sistema.fila_exit, thread);
}

TCB* escalonar(void) {
    return remover_fila(&sistema.fila_ready);
}

void ativar_dispatcher(void) {
    sistema.dispatcher_ativo = true;
}

void mover_todas_new_para_ready(void) {
    TCB *atual = sistema.fila_new.inicio;
    TCB *seguinte;
    while (atual) {
        seguinte = atual->seguinte;
        if (atual->tempo_chegada_new < sistema.tempo_atual) {
            remover_thread_da_fila(&sistema.fila_new, atual);
            mover_para_ready(atual);
        }
        atual = seguinte;
    }
}

void limpar_threads_exit(void) {
    TCB *thread;
    while ((thread = remover_fila(&sistema.fila_exit)) != NULL) {
        PCB *proc = sistema.processos;
        while (proc && proc->id_processo != thread->id_processo) proc = proc->seguinte;
        if (proc) {
            if (proc->threads == thread) {
                proc->threads = thread->seguinte_proc;
            } else {
                TCB *t = proc->threads;
                while (t && t->seguinte_proc != thread) t = t->seguinte_proc;
                if (t) t->seguinte_proc = thread->seguinte_proc;
            }
            proc->total_threads--;
        }
        if (thread->numero_pagina >= 0) libertar_pagina(thread->numero_pagina);
        free(thread);
    }
}

void atualizar_threads_bloqueadas(void) {
    TCB *atual = sistema.fila_blocked.inicio;
    TCB *seguinte;
    
    while (atual) {
        seguinte = atual->seguinte;
        atual->tempo_bloqueado--;
        
        if (atual->tempo_bloqueado <= 0) {
            remover_thread_da_fila(&sistema.fila_blocked, atual);
            mover_para_ready(atual);
        }
        atual = seguinte;
    }
}

void matar_todas_threads_no_processo(PCB *processo) {
    if (!processo) return;
    
    TCB *atual = processo->threads;
    TCB *novo_inicio = NULL;
    TCB *novo_fim = NULL;
    int novo_total = 0;
    
    while (atual) {
        TCB *seguinte = atual->seguinte_proc;
        
        if (atual->estado == EXIT) {
            if (!novo_inicio) {
                novo_inicio = atual;
            } else {
                novo_fim->seguinte_proc = atual;
            }
            novo_fim = atual;
            atual->seguinte_proc = NULL;
            novo_total++;
        } else {
            if (atual->estado == READY) {
                remover_thread_da_fila(&sistema.fila_ready, atual);
            } else if (atual->estado == BLOCKED) {
                remover_thread_da_fila(&sistema.fila_blocked, atual);
            } else if (atual->estado == RUN) {
                sistema.thread_em_execucao = NULL;
            }
            
            if (atual->numero_pagina >= 0) {
                libertar_pagina(atual->numero_pagina);
            }
            
            free(atual);
        }
        
        atual = seguinte;
    }
    
    processo->threads = novo_inicio;
    processo->total_threads = novo_total;
}

void destruir_processo(PCB *proc) {
    if (!proc) return;
    
    matar_todas_threads_no_processo(proc);
    
    if (proc->numero_pagina >= 0) {
        libertar_pagina(proc->numero_pagina);
    }
    
    if (sistema.processos == proc) {
        sistema.processos = proc->seguinte;
    } else {
        PCB *atual = sistema.processos;
        while (atual && atual->seguinte != proc) {
            atual = atual->seguinte;
        }
        if (atual) {
            atual->seguinte = proc->seguinte;
        }
    }
    
    free(proc);
}

void destruir_sistema(void) {
    while (sistema.processos) {
        PCB *temp = sistema.processos;
        sistema.processos = sistema.processos->seguinte;
        destruir_processo(temp);
    }
    
    limpar_fila(&sistema.fila_new);
    limpar_fila(&sistema.fila_ready);
    limpar_fila(&sistema.fila_blocked);
    limpar_fila(&sistema.fila_exit);
    
    sistema.thread_em_execucao = NULL;
    sistema.total_processos = 0;
}

int recolher_todas_new_para_ready(TCB *recolhidas[], int tamanho_maximo) {
    TCB *atual = sistema.fila_new.inicio;
    TCB *seguinte;
    int contador = 0;
    while (atual && contador < tamanho_maximo) {
        seguinte = atual->seguinte;
        if (atual->tempo_chegada_new < sistema.tempo_atual) {
            remover_thread_da_fila(&sistema.fila_new, atual);
            mudar_estado_thread(atual, READY);
            atual->tempo_cpu_usado = 0;
            recolhidas[contador++] = atual;
        }
        atual = seguinte;
    }
    return contador;
}

int recolher_threads_bloqueadas_para_ready(TCB *recolhidas[], int tamanho_maximo) {
    TCB *atual = sistema.fila_blocked.inicio;
    TCB *seguinte;
    int contador = 0;
    
    while (atual && contador < tamanho_maximo) {
        seguinte = atual->seguinte;
        atual->tempo_bloqueado--;
        
        if (atual->tempo_bloqueado <= 0) {
            remover_thread_da_fila(&sistema.fila_blocked, atual);
            mudar_estado_thread(atual, READY);
            atual->tempo_cpu_usado = 0;
            recolhidas[contador++] = atual;
        }
        atual = seguinte;
    }
    return contador;
}