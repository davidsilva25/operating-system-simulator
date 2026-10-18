#include "memoria.h"

#define TOTAL_PAGINAS 20 
#define TAMANHO_MEMORIA_PAGINA (TAMANHO_PAGINA * sizeof(int))

static int memoria[TOTAL_PAGINAS * TAMANHO_PAGINA];
static bool pagina_alocada[TOTAL_PAGINAS] = {0};

void inicializar_memoria(void) {
    memset(memoria, 0, sizeof(memoria));
    memset(pagina_alocada, 0, sizeof(pagina_alocada));
}

int alocar_pagina(void) {
    for (int i = 0; i < TOTAL_PAGINAS; i++) {
        if (!pagina_alocada[i]) {
            pagina_alocada[i] = true;
            return i;
        }
    }
    return -1;
}

void libertar_pagina(int numero_pagina) {
    if (numero_pagina >= 0 && numero_pagina < TOTAL_PAGINAS) {
        pagina_alocada[numero_pagina] = false;
        memset(&memoria[numero_pagina * TAMANHO_PAGINA], 0, TAMANHO_MEMORIA_PAGINA);
    }
}

void definir_var_global(PCB *processo, int indice_var, int valor) {
    if (indice_var >= 0 && indice_var < MAX_VARS_GLOBAIS) {
        if (processo->numero_pagina >= 0) {
            memoria[processo->numero_pagina * TAMANHO_PAGINA + indice_var] = valor;
        }
        processo->vars_globais.valores[indice_var] = valor;
    }
}

int obter_var_global(PCB *processo, int indice_var) {
    if (indice_var >= 0 && indice_var < MAX_VARS_GLOBAIS) {
        if (processo->numero_pagina >= 0) {
            return memoria[processo->numero_pagina * TAMANHO_PAGINA + indice_var];
        }
        return processo->vars_globais.valores[indice_var];
    }
    return 0;
}

void definir_var_local(TCB *thread, int indice_var, int valor) {
    if (indice_var >= 0 && indice_var < MAX_VARS_LOCAIS) {
        if (thread->numero_pagina >= 0) {
            memoria[thread->numero_pagina * TAMANHO_PAGINA + indice_var] = valor;
        }
        thread->vars_locais.valores[indice_var] = valor;
    }
}

int obter_var_local(TCB *thread, int indice_var) {
    if (indice_var >= 0 && indice_var < MAX_VARS_LOCAIS) {
        if (thread->numero_pagina >= 0) {
            return memoria[thread->numero_pagina * TAMANHO_PAGINA + indice_var];
        }
        return thread->vars_locais.valores[indice_var];
    }
    return 0;
}

void escrever_memoria(int numero_pagina, int offset, int valor) {
    if (numero_pagina >= 0 && offset >= 0 && offset < TAMANHO_PAGINA) {
        memoria[numero_pagina * TAMANHO_PAGINA + offset] = valor;
    }
}

int ler_memoria(int numero_pagina, int offset) {
    if (numero_pagina >= 0 && offset >= 0 && offset < TAMANHO_PAGINA) {
        return memoria[numero_pagina * TAMANHO_PAGINA + offset];
    }
    return 0;
}