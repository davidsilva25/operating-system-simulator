#include "inputs.h"
#include <ctype.h>
#include "memoria.h"

TipoInstrucao obter_tipo_instrucao(const char *str) {
    if (strcmp(str, "WORK") == 0) return WORK;
    if (strcmp(str, "BLOCK") == 0) return BLOCK;
    if (strcmp(str, "SET") == 0) return SET;
    if (strcmp(str, "ADD") == 0) return ADD;
    if (strcmp(str, "PRINT") == 0) return PRINT;
    if (strcmp(str, "JUMP") == 0) return JUMP;
    if (strcmp(str, "JZ") == 0) return JZ;
    if (strcmp(str, "PROC_CREATE") == 0) return PROC_CREATE;
    if (strcmp(str, "THREAD_CREATE") == 0) return THREAD_CREATE;
    if (strcmp(str, "END") == 0) return END;
    return -1;
}

int interpretar_variavel(const char *str) {
    if (str[0] == 'G' && isdigit(str[1])) return (str[1] - '0');
    if (str[0] == 'L' && isdigit(str[1])) return (str[1] - '0') + 10;
    return atoi(str);
}

Instrucao interpretar_instrucao(const char *linha) {
    Instrucao instr;
    char tipo_str[32];
    
    while (*linha && isspace(*linha)) linha++;
    
    sscanf(linha, "%31s", tipo_str);
    instr.tipo = obter_tipo_instrucao(tipo_str);
    instr.arg1 = 0;
    instr.arg2 = 0;
    
    if (instr.tipo == BLOCK) {
        sscanf(linha, "%*s %d", &instr.arg1);
    } else if (instr.tipo == SET || instr.tipo == ADD) {
        char var_str[32];
        sscanf(linha, "%*s %31s %d", var_str, &instr.arg2);
        instr.arg1 = interpretar_variavel(var_str);
    } else if (instr.tipo == PRINT) {
        char var_str[32];
        sscanf(linha, "%*s %31s", var_str);
        instr.arg1 = interpretar_variavel(var_str);
    } else if (instr.tipo == JUMP) {
        sscanf(linha, "%*s %d", &instr.arg2); 
    } else if (instr.tipo == JZ) {
        char var_str[32];
        sscanf(linha, "%*s %31s %d", var_str, &instr.arg2);
        instr.arg1 = interpretar_variavel(var_str);
    } else if (instr.tipo == PROC_CREATE || instr.tipo == THREAD_CREATE) {
        sscanf(linha, "%*s %31s", instr.arg_str);
        instr.arg1 = atoi(instr.arg_str);
    }
    
    return instr;
}

char* instrucao_para_string(Instrucao instr) {
    static char buffer[256];
    
    switch (instr.tipo) {
        case WORK:
            sprintf(buffer, "WORK");
            break;
        case BLOCK:
            sprintf(buffer, "BLOCK %d", instr.arg1);
            break;
        case SET:
            if (instr.arg1 < 0)
                sprintf(buffer, "SET G%d %d", -instr.arg1, instr.arg2);
            else
                sprintf(buffer, "SET L%d %d", instr.arg1, instr.arg2);
            break;
        case ADD:
            if (instr.arg1 < 0)
                sprintf(buffer, "ADD G%d %d", -instr.arg1, instr.arg2);
            else
                sprintf(buffer, "ADD L%d %d", instr.arg1, instr.arg2);
            break;
        case PRINT:
            if (instr.arg1 < 0)
                sprintf(buffer, "PRINT G%d", -instr.arg1);
            else
                sprintf(buffer, "PRINT L%d", instr.arg1);
            break;
        case JUMP:
            sprintf(buffer, "JUMP %d", instr.arg2); 
            break;
        case JZ:
            if (instr.arg1 < 0)
                sprintf(buffer, "JZ G%d %d", -instr.arg1, instr.arg2);
            else
                sprintf(buffer, "JZ L%d %d", instr.arg1, instr.arg2);
            break;
        case PROC_CREATE:
            sprintf(buffer, "PROC_CREATE %s", instr.arg_str);
            break;
        case THREAD_CREATE:
            sprintf(buffer, "THREAD_CREATE %s", instr.arg_str);
            break;
        case END:
            sprintf(buffer, "END");
            break;
    }
    
    return buffer;
}

int empacotar_instrucao(TipoInstrucao tipo, int arg1, int arg2) {
    int arg2_codificado = arg2 + 50;
    return (tipo * 10000) + (arg1 * 100) + arg2_codificado;
}

Instrucao desempacotar_instrucao(int empacotada) {
    Instrucao instr;
    instr.tipo = empacotada / 10000;
    instr.arg1 = (empacotada / 100) % 100;
    instr.arg2 = (empacotada % 100) - 50;
    
    if (instr.tipo == PROC_CREATE || instr.tipo == THREAD_CREATE) {
        sprintf(instr.arg_str, "%02d", instr.arg1);
    }
    return instr;
}

int carregar_programa(const char *nome_ficheiro, Programa *prog) {
    FILE *ficheiro = fopen(nome_ficheiro, "r");
    if (!ficheiro) {
        fprintf(stderr, "Erro ao abrir ficheiro: %s\n", nome_ficheiro);
        return 0;
    }
    
    char linha[256];
    prog->total_instrucoes = 0;
    
    prog->numeros_pagina[0] = alocar_pagina();
    prog->numeros_pagina[1] = alocar_pagina();
    prog->numeros_pagina[2] = alocar_pagina();
    
    const char *nome = strrchr(nome_ficheiro, '/');
    nome = nome ? nome + 1 : nome_ficheiro;
    strcpy(prog->nome, nome);
    if (strchr(prog->nome, '.')) *strchr(prog->nome, '.') = '\0';
    
    while (fgets(linha, sizeof(linha), ficheiro) && prog->total_instrucoes < MAX_INSTRUCOES) {
        if (linha[0] == '#' || linha[0] == '\n' || linha[0] == '\0') continue;
        
        Instrucao instr = interpretar_instrucao(linha);
        
        int empacotada = empacotar_instrucao(instr.tipo, instr.arg1, instr.arg2);
        
        int id_pagina = prog->total_instrucoes / TAMANHO_PAGINA;
        int offset = prog->total_instrucoes % TAMANHO_PAGINA;
        escrever_memoria(prog->numeros_pagina[id_pagina], offset, empacotada);
        
        prog->total_instrucoes++;
    }
    
    fclose(ficheiro);
    return 1;
}

void carregar_todos_programas(const char *diretorio __attribute__((unused))) {
}

Programa* obter_programa(const char *nome) {
    for (int i = 0; i < sistema.total_programas; i++) {
        if (strcmp(sistema.programas[i].nome, nome) == 0) {
            return &sistema.programas[i];
        }
    }
    return NULL;
}