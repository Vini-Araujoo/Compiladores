#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/tabela.h"

int main(void) {
    printf("Executando teste_pilha_escopos...\n");

    /* 1. Inicializacao e Escopo Global (Nivel 0) */
    inicializarTabelaSimbolos();
    Escopo *global = obterEscopoAtual();
    assert(global != NULL);
    assert(global->nivel == 0);
    assert(global->pai == NULL);

    Modificadores mods_padrao = MODIFICADORES_PADRAO;
    Modificadores mods_const = {0, 0, 0, 0, 1}; /* final */
    mods_const.eh_public = 1;

    /* Insercoes no escopo global */
    assert(inserirSimbolo("g_int", TIPO_INT, mods_padrao, 1) == 1);
    assert(inserirSimbolo("g_double", TIPO_DOUBLE, mods_padrao, 2) == 1);
    assert(inserirSimbolo("PI", TIPO_FLOAT, mods_const, 3) == 1);

    Simbolo *sg1 = buscarSimbolo("g_int");
    assert(sg1 != NULL);
    assert(strcmp(sg1->nome, "g_int") == 0);
    assert(sg1->tipo == TIPO_INT);
    assert(sg1->linha == 1);

    Simbolo *sg2 = buscarSimbolo("PI");
    assert(sg2 != NULL);
    assert(sg2->mods.eh_final == 1);
    assert(sg2->mods.eh_public == 1);

    /* 2. Empilhar Escopo de Metodo (Nivel 1) */
    empilharEscopo();
    Escopo *metodo = obterEscopoAtual();
    assert(metodo != NULL);
    assert(metodo->nivel == 1);
    assert(metodo->pai == global);

    assert(inserirSimbolo("param_x", TIPO_STRING, mods_padrao, 10) == 1);
    assert(inserirSimbolo("flag", TIPO_BOOLEAN, mods_padrao, 11) == 1);

    /* Variaveis do metodo acessiveis */
    assert(buscarSimbolo("param_x") != NULL);
    assert(buscarSimbolo("flag") != NULL);

    /* Busca ascendente: variaveis globais permanecem visiveis */
    assert(buscarSimbolo("g_int") != NULL);
    assert(buscarSimbolo("g_double") != NULL);
    assert(buscarSimbolo("PI") != NULL);

    /* 3. Sombreamento (Shadowing) de variavel global no escopo local */
    assert(inserirSimbolo("g_int", TIPO_SHORT, mods_padrao, 12) == 1);
    Simbolo *s_sombreado = buscarSimbolo("g_int");
    assert(s_sombreado != NULL);
    assert(s_sombreado->tipo == TIPO_SHORT); /* Enxerga a versao local mascarando a global */
    assert(s_sombreado->linha == 12);

    /* 4. Empilhar Bloco Aninhado (Nivel 2) */
    empilharEscopo();
    Escopo *bloco = obterEscopoAtual();
    assert(bloco != NULL);
    assert(bloco->nivel == 2);
    assert(bloco->pai == metodo);

    assert(inserirSimbolo("bloco_temp", TIPO_BYTE, mods_padrao, 20) == 1);
    assert(inserirSimbolo("g_int", TIPO_CHAR, mods_padrao, 21) == 1);

    /* No nivel 2, g_int e TIPO_CHAR */
    Simbolo *s_nivel2 = buscarSimbolo("g_int");
    assert(s_nivel2 != NULL);
    assert(s_nivel2->tipo == TIPO_CHAR);
    assert(s_nivel2->linha == 21);

    assert(buscarSimbolo("bloco_temp") != NULL);
    assert(buscarSimbolo("param_x") != NULL);
    assert(buscarSimbolo("g_double") != NULL);

    /* 5. Desempilhar Bloco (retorna ao Nivel 1) */
    desempilharEscopo();
    assert(obterEscopoAtual() == metodo);
    assert(obterEscopoAtual()->nivel == 1);

    /* bloco_temp nao existe mais */
    assert(buscarSimbolo("bloco_temp") == NULL);

    /* g_int voltou a ser o do metodo (TIPO_SHORT) */
    Simbolo *s_pos_bloco = buscarSimbolo("g_int");
    assert(s_pos_bloco != NULL);
    assert(s_pos_bloco->tipo == TIPO_SHORT);
    assert(s_pos_bloco->linha == 12);

    /* 6. Desempilhar Metodo (retorna ao Nivel 0 - Global) */
    desempilharEscopo();
    assert(obterEscopoAtual() == global);
    assert(obterEscopoAtual()->nivel == 0);

    /* param_x e flag nao existem mais */
    assert(buscarSimbolo("param_x") == NULL);
    assert(buscarSimbolo("flag") == NULL);

    /* g_int voltou a ser o original global (TIPO_INT) */
    Simbolo *s_global_orig = buscarSimbolo("g_int");
    assert(s_global_orig != NULL);
    assert(s_global_orig->tipo == TIPO_INT);
    assert(s_global_orig->linha == 1);

    /* 7. Liberacao Total da Tabela */
    liberarTabelaSimbolos();
    assert(obterEscopoAtual() == NULL);
    assert(buscarSimbolo("g_int") == NULL);

    printf("teste_pilha_escopos passou com sucesso!\n");
    return 0;
}
