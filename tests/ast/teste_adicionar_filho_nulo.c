#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../../src/ast.h"

int main(void) {
    printf("Executando teste_adicionar_filho_nulo...\n");

    NoAST *pai = criarNo(NO_BLOCO, 10);
    assert(pai != NULL);
    assert(pai->num_filhos == 0);

    /* 1. Um filho NULL deve ocupar uma posicao no vetor. */
    adicionarFilho(pai, NULL);
    assert(pai->num_filhos == 1);
    assert(pai->filhos[0] == NULL);

    /* 2. Tentar adicionar filho válido a um pai NULL (deve ignorar com segurança) */
    NoAST *filho = criarNo(NO_RETURN, 11);
    assert(filho != NULL);
    adicionarFilho(NULL, filho);

    /* 3. Tentar adicionar NULL a NULL */
    adicionarFilho(NULL, NULL);

    /* 4. O filho seguinte deve manter o indice correto. */
    adicionarFilho(pai, filho);
    assert(pai->num_filhos == 2);
    assert(pai->filhos[0] == NULL);
    assert(pai->filhos[1] == filho);

    liberarAST(pai);

    printf("teste_adicionar_filho_nulo passou com sucesso!\n");
    return 0;
}
