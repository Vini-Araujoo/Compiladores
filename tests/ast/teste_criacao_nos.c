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

    /* 4. Liberar árvore completa */
    liberarAST(raiz);

    printf("teste_criacao_nos passou com sucesso!\n");
    return 0;
}