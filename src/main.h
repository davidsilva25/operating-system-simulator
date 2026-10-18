#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_PROCESSOS 100
#define MAX_THREADS 100
#define MAX_INSTRUCOES 15
#define MAX_VARS_GLOBAIS 5
#define MAX_VARS_LOCAIS 5
#define TAMANHO_PAGINA 5
#define QUANTUM 3
#define MAX_UNIDADES_TEMPO 100

typedef enum {
    NEW,
    READY,
    RUN,
    BLOCKED,
    EXIT
} EstadoThread;

typedef enum {
    WORK,
    BLOCK,
    SET,
    ADD,
    PRINT,
    JUMP,
    JZ,
    PROC_CREATE,
    THREAD_CREATE,
    END
} TipoInstrucao;

typedef struct {
    TipoInstrucao tipo;
    int arg1;
    int arg2;
    char arg_str[32];
} Instrucao;

typedef struct {
    char nome[32];
    int total_instrucoes;
    int numeros_pagina[3];
} Programa;

typedef struct {
    int valores[MAX_VARS_GLOBAIS];
} VariaveisGlobais;

typedef struct {
    int valores[MAX_VARS_LOCAIS];
} VariaveisLocais;

typedef struct ThreadControlBlock TCB;
typedef struct ProcessControlBlock PCB;

typedef struct ThreadControlBlock {
    int id_processo;
    int id_thread;
    EstadoThread estado;
    int contador_programa;
    VariaveisLocais vars_locais;
    int tempo_cpu_usado;
    int tempo_bloqueado;
    int tempo_chegada_new;
    Programa *programa;
    struct ThreadControlBlock *seguinte;
    struct ThreadControlBlock *seguinte_proc;
    int numero_pagina;
} TCB;

typedef struct ProcessControlBlock {
    int id_processo;
    VariaveisGlobais vars_globais;
    TCB *threads;
    int total_threads;
    struct ProcessControlBlock *seguinte;
    int numero_pagina;
} PCB;

typedef struct {
    TCB *inicio;
    TCB *fim;
} Fila;

typedef struct {
    PCB *processos;
    int total_processos;
    Fila fila_new;
    Fila fila_ready;
    TCB *thread_em_execucao;
    Fila fila_blocked;
    Fila fila_exit;
    Programa programas[50];
    int total_programas;
    int tempo_atual;
    bool dispatcher_ativo;
} Sistema;

extern Sistema sistema;

#endif