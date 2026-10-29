#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela.h"

static Escopo *escopo_atual = NULL;

/*
 * Simbolo mockado para stub provisorio, evitando o bloqueio dos colegas
 * enquanto a pilha real de escopos esta sendo programada.
 */
static Simbolo simbolo_mock = {
    .nome = "mock",
    .tipo = TIPO_INT,
    .mods = {0, 0, 0, 0, 0},
    .linha = 1,
    .proximo = NULL
};

void inicializarTabelaSimbolos(void) {
    /* Stub inicial */
    escopo_atual = NULL;
}

void empilharEscopo(void) {
    /* Stub inicial */
}

void desempilharEscopo(void) {
    /* Stub inicial */
}

int inserirSimbolo(char *nome, TipoDado tipo, Modificadores mods, int linha) {
    /* Stub inicial provisorio */
    (void)nome;
    (void)tipo;
    (void)mods;
    (void)linha;
    return 1;
}

Simbolo *buscarSimbolo(char *nome) {
    /* Stub provisorio: retorna simbolo mockado com TIPO_INT */
    (void)nome;
    return &simbolo_mock;
}

Simbolo *buscarSimboloEscopoAtual(char *nome) {
    (void)nome;
    return &simbolo_mock;
}

Escopo *obterEscopoAtual(void) {
    return escopo_atual;
}

void liberarTabelaSimbolos(void) {
    /* Stub inicial */
    escopo_atual = NULL;
}
