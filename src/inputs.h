#ifndef INPUTS_H
#define INPUTS_H

#include "main.h"

TipoInstrucao obter_tipo_instrucao(const char *str);
int interpretar_variavel(const char *str);
Instrucao interpretar_instrucao(const char *linha);
char* instrucao_para_string(Instrucao instr);

int empacotar_instrucao(TipoInstrucao tipo, int arg1, int arg2);
Instrucao desempacotar_instrucao(int empacotada);

int carregar_programa(const char *nome_ficheiro, Programa *prog);
void carregar_todos_programas(const char *diretorio);
Programa* obter_programa(const char *nome);

#endif