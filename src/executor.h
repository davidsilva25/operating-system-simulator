#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "main.h"

void executar_instrucao(TCB *thread, Instrucao instr);
void executar_ciclo_cpu(void);

#endif