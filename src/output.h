#ifndef OUTPUT_H
#define OUTPUT_H

#include "main.h"

void imprimir_estado_sistema(void);
void imprimir_marcador_tempo(int tempo);
void limpar_buffer_saida(void);
void registar_instrucao_print(TCB *thread, const char *valor);

#endif