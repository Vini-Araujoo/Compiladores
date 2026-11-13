#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela.h"

static Escopo *escopo_atual = NULL;
static Escopo *todos_escopos = NULL;

static unsigned int hashString(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return (unsigned int)(hash % TAM_TABELA_HASH);
}

void inicializarTabelaSimbolos(void) {
    liberarTabelaSimbolos();
    empilharEscopo();
}

void empilharEscopo(void) {
    Escopo *novo = (Escopo *)malloc(sizeof(Escopo));
    if (!novo) {
        perror("Erro ao alocar novo Escopo");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < TAM_TABELA_HASH; i++) {
        novo->tabela[i] = NULL;
    }

    novo->pai = escopo_atual;
    novo->nivel = (escopo_atual != NULL) ? escopo_atual->nivel + 1 : 0;
    novo->proximo_alocado = todos_escopos;
    todos_escopos = novo;
    escopo_atual = novo;
}

void desempilharEscopo(void) {
    if (!escopo_atual) {
        return;
    }
    escopo_atual = escopo_atual->pai;
}

int inserirSimbolo(char *nome, TipoDado tipo, Modificadores mods, int linha) {
    if (!nome) {
        return 0;
    }

    if (!escopo_atual) {
        empilharEscopo();
    }

    /* Validacao no escopo corrente: proibir redeclaracao com mesmo identificador */
    if (buscarSimboloEscopoAtual(nome) != NULL) {
        return 0;
    }

    unsigned int indice = hashString(nome);
    Simbolo *novo = (Simbolo *)malloc(sizeof(Simbolo));
    if (!novo) {
        perror("Erro ao alocar Simbolo");
        exit(EXIT_FAILURE);
    }

    novo->nome = strdup(nome);
    novo->tipo = tipo;
    novo->mods = mods;
    novo->eh_final = mods.eh_final;
    novo->linha = linha;
    novo->proximo = escopo_atual->tabela[indice];
    escopo_atual->tabela[indice] = novo;

    return 1;
}

Simbolo *buscarSimbolo(char *nome) {
    if (!nome || !escopo_atual) {
        return NULL;
    }

    /* Busca ascendente: do escopo local ate o escopo mais externo / global */
    Escopo *escopo = escopo_atual;
    while (escopo != NULL) {
        unsigned int indice = hashString(nome);
        Simbolo *simb = escopo->tabela[indice];

        while (simb != NULL) {
            if (strcmp(simb->nome, nome) == 0) {
                return simb;
            }
            simb = simb->proximo;
        }

        escopo = escopo->pai;
    }

    return NULL;
}

Simbolo *buscarSimboloEscopoAtual(char *nome) {
    if (!nome || !escopo_atual) {
        return NULL;
    }

    unsigned int indice = hashString(nome);
    Simbolo *simb = escopo_atual->tabela[indice];

    while (simb != NULL) {
        if (strcmp(simb->nome, nome) == 0) {
            return simb;
        }
        simb = simb->proximo;
    }

    return NULL;
}

Escopo *obterEscopoAtual(void) {
    return escopo_atual;
}

void liberarTabelaSimbolos(void) {
    escopo_atual = NULL;
    Escopo *esc = todos_escopos;
    while (esc) {
        Escopo *prox = esc->proximo_alocado;
        for (int i = 0; i < TAM_TABELA_HASH; i++) {
            Simbolo *simb = esc->tabela[i];
            while (simb) {
                Simbolo *proximo = simb->proximo;
                free(simb->nome);
                free(simb);
                simb = proximo;
            }
        }
        free(esc);
        esc = prox;
    }
    todos_escopos = NULL;
}
