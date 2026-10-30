#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../../src/ast.h"

int main(void) {
    printf("Executando teste_impressao_arvore_vazia...\n");

    /* 1. Validar que passar ponteiro NULL nao causa Segmentation Fault */
    imprimirAST(NULL);

    printf("teste_impressao_arvore_vazia passou com sucesso!\n");
    return 0;
}
