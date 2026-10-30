#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/tabela.h"

int main(void) {
    printf("Executando teste_busca_apos_fechamento_escopo...\n");

    inicializarTabelaSimbolos();
    Modificadores mods = MODIFICADORES_PADRAO;

    /* 1. Inserir simbolo no escopo raiz (nivel 0) */
    assert(inserirSimbolo("var_global", TIPO_INT, mods, 1) == 1);
    assert(buscarSimbolo("var_global") != NULL);

    /* 2. Criar bloco interno (nivel 1) e declarar variavel efemera */
    empilharEscopo();
    assert(obterEscopoAtual()->nivel == 1);

    assert(inserirSimbolo("var_local_bloco", TIPO_STRING, mods, 5) == 1);
    assert(buscarSimbolo("var_local_bloco") != NULL);

    /* 3. Fechar bloco interno */
    desempilharEscopo();
    assert(obterEscopoAtual()->nivel == 0);

    /* CASO NEGATIVO 1: A variavel local deve estar absolutamente inacessivel */
    Simbolo *busca_invalida = buscarSimbolo("var_local_bloco");
    if (busca_invalida != NULL) {
        fprintf(stderr, "Erro de isolamento de escopo na linha 5: simbolo 'var_local_bloco' permaneceu visivel apos desempilhamento!\n");
        return 1;
    }
    assert(busca_invalida == NULL);
    printf("  [PASS] var_local_bloco invisivel apos fechamento do escopo\n");

    /* CASO NEGATIVO 2: Rejeicao de redeclaracao de variavel no mesmo escopo corrente */
    assert(inserirSimbolo("duplicada", TIPO_DOUBLE, mods, 10) == 1);

    /* Tentativa de redeclarar 'duplicada' no mesmo escopo (linha 12) */
    int resultado_redecl = inserirSimbolo("duplicada", TIPO_INT, mods, 12);
    if (resultado_redecl != 0) {
        fprintf(stderr, "Falha: redeclaracao de 'duplicada' no mesmo escopo foi indevidamente aceita!\n");
        return 1;
    }
    assert(resultado_redecl == 0);
    fprintf(stderr, "Erro semantico na linha 12: redeclaracao invalida da variavel 'duplicada' no mesmo escopo (declarada anteriormente na linha 10).\n");
    printf("  [PASS] Redeclaracao de 'duplicada' rejeitada com sucesso\n");

    /* CASO NEGATIVO 3: Busca apos liberacao total da tabela */
    liberarTabelaSimbolos();
    assert(obterEscopoAtual() == NULL);
    assert(buscarSimbolo("var_global") == NULL);
    assert(buscarSimbolo("duplicada") == NULL);
    printf("  [PASS] Busca em tabela liberada retorna NULL\n");

    printf("teste_busca_apos_fechamento_escopo passou com sucesso!\n");
    return 0;
}
