#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/ast.h"

int main(void) {
    printf("Executando teste_criacao_nos...\n");

    /* 1. Criar nó raiz */
    NoAST *raiz = criarNo(NO_PROGRAMA, 1);
    assert(raiz != NULL);
    assert(raiz->tipo_no == NO_PROGRAMA);
    assert(raiz->linha == 1);
    assert(raiz->num_filhos == 0);
    assert(raiz->capacidade >= 4);

    /* 2. Criar e adicionar nó de classe */
    NoAST *classe = criarNo(NO_CLASSE, 2);
    classe->valor.valor_str = strdup("MinhaClasse");
    adicionarFilho(raiz, classe);
    assert(raiz->num_filhos == 1);
    assert(raiz->filhos[0] == classe);

    /* 3. Adicionar múltiplos filhos para testar redimensionamento dinâmico (N filhos > capacidade inicial) */
    for (int i = 0; i < 10; i++) {
        NoAST *metodo = criarNo(NO_METODO, 3 + i);
        adicionarFilho(classe, metodo);
    }

    assert(classe->num_filhos == 10);
    assert(classe->capacidade >= 10);

    for (int i = 0; i < 10; i++) {
        assert(classe->filhos[i]->tipo_no == NO_METODO);
        assert(classe->filhos[i]->linha == 3 + i);
    }

    /* 4. Testar nós de parâmetros e parâmetro */
    NoAST *params = criarNo(NO_PARAMETROS, 15);
    NoAST *p1 = criarNo(NO_PARAMETRO, 15);
    p1->tipo_dado = TIPO_INT;
    p1->valor.valor_str = strdup("x");
    adicionarFilho(params, p1);
    assert(params->num_filhos == 1);
    assert(params->filhos[0]->tipo_no == NO_PARAMETRO);
    assert(strcmp(params->filhos[0]->valor.valor_str, "x") == 0);
    adicionarFilho(classe->filhos[0], params);

    /* 5. Testar nós de declaração de variáveis (NO_DECL_VARIAVEL) */
    NoAST *decl1 = criarNo(NO_DECL_VARIAVEL, 20);
    decl1->tipo_dado = TIPO_INT;
    decl1->valor.valor_str = strdup("a");
    NoAST *lit1 = criarNo(NO_LITERAL_INT, 20);
    lit1->valor.valor_int = 1;
    adicionarFilho(decl1, lit1);
    assert(decl1->tipo_no == NO_DECL_VARIAVEL);
    assert(decl1->tipo_dado == TIPO_INT);
    assert(strcmp(decl1->valor.valor_str, "a") == 0);
    assert(decl1->num_filhos == 1);
    assert(decl1->filhos[0] == lit1);

    /* Declaração sem valor inicial */
    NoAST *decl2 = criarNo(NO_DECL_VARIAVEL, 21);
    decl2->tipo_dado = TIPO_DOUBLE;
    decl2->valor.valor_str = strdup("b");
    assert(decl2->num_filhos == 0);

    /* Declaração com filho de inicialização NULL */
    NoAST *decl3 = criarNo(NO_DECL_VARIAVEL, 22);
    decl3->tipo_dado = TIPO_BOOLEAN;
    decl3->valor.valor_str = strdup("c");
    adicionarFilho(decl3, NULL);
    assert(decl3->num_filhos == 1);
    assert(decl3->filhos[0] == NULL);

    adicionarFilho(classe, decl1);
    adicionarFilho(classe, decl2);
    adicionarFilho(classe, decl3);

    /* 6. Testar nós de atribuição (NO_ATRIBUICAO: simples e composta) */
    NoAST *atrib_simples = criarNo(NO_ATRIBUICAO, 23);
    strncpy(atrib_simples->valor.operador, "=", sizeof(atrib_simples->valor.operador) - 1);
    NoAST *id_x = criarNo(NO_IDENTIFICADOR, 23);
    id_x->valor.valor_str = strdup("x");
    NoAST *lit2 = criarNo(NO_LITERAL_INT, 23);
    lit2->valor.valor_int = 10;
    adicionarFilho(atrib_simples, id_x);
    adicionarFilho(atrib_simples, lit2);
    assert(strcmp(atrib_simples->valor.operador, "=") == 0);
    assert(atrib_simples->num_filhos == 2);
    assert(atrib_simples->filhos[0] == id_x);

    NoAST *atrib_composta_soma = criarNo(NO_ATRIBUICAO, 24);
    strncpy(atrib_composta_soma->valor.operador, "+=", sizeof(atrib_composta_soma->valor.operador) - 1);
    assert(strcmp(atrib_composta_soma->valor.operador, "+=") == 0);

    NoAST *atrib_composta_sub = criarNo(NO_ATRIBUICAO, 25);
    strncpy(atrib_composta_sub->valor.operador, "-=", sizeof(atrib_composta_sub->valor.operador) - 1);
    assert(strcmp(atrib_composta_sub->valor.operador, "-=") == 0);

    adicionarFilho(classe, atrib_simples);
    adicionarFilho(classe, atrib_composta_soma);
    adicionarFilho(classe, atrib_composta_sub);

    /* 7. Liberar árvore completa */
    liberarAST(raiz);

    printf("teste_criacao_nos passou com sucesso!\n");
    return 0;
}