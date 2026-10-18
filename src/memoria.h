#ifndef MEMORIA_H
#define MEMORIA_H

#include "main.h"

void inicializar_memoria(void);
int alocar_pagina(void);
void libertar_pagina(int numero_pagina);
void definir_var_global(PCB *processo, int indice_var, int valor);
int obter_var_global(PCB *processo, int indice_var);
void definir_var_local(TCB *thread, int indice_var, int valor);
int obter_var_local(TCB *thread, int indice_var);
void escrever_memoria(int numero_pagina, int offset, int valor);
int ler_memoria(int numero_pagina, int offset);

#endif