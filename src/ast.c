#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

#define CAPACIDADE_INICIAL_FILHOS 4

NoAST *criarNo(TipoNoAST tipo, int linha) {
    NoAST *no = (NoAST *)malloc(sizeof(NoAST));
    if (!no) {
        perror("Erro ao alocar memoria para NoAST");
        exit(EXIT_FAILURE);
    }

    no->tipo_no = tipo;
    no->tipo_dado = TIPO_VOID;
    no->linha = linha;
    no->num_filhos = 0;
    no->capacidade = CAPACIDADE_INICIAL_FILHOS;
    memset(&no->valor, 0, sizeof(ValorLiteral));

    no->filhos = (NoAST **)malloc(sizeof(NoAST *) * no->capacidade);
    if (!no->filhos) {
        perror("Erro ao alocar vetor de filhos");
        free(no);
        exit(EXIT_FAILURE);
    }

    return no;
}

void adicionarFilho(NoAST *pai, NoAST *filho) {
    if (!pai) {
        return;
    }

    if (pai->num_filhos >= pai->capacidade) {
        int nova_capacidade = (pai->capacidade == 0) ? CAPACIDADE_INICIAL_FILHOS : pai->capacidade * 2;
        NoAST **novos_filhos = (NoAST **)realloc(pai->filhos, sizeof(NoAST *) * nova_capacidade);
        if (!novos_filhos) {
            perror("Erro ao redimensionar vetor de filhos");
            exit(EXIT_FAILURE);
        }
        pai->filhos = novos_filhos;
        pai->capacidade = nova_capacidade;
    }

    pai->filhos[pai->num_filhos++] = filho;
}

void liberarAST(NoAST *raiz) {
    if (!raiz) {
        return;
    }

    for (int i = 0; i < raiz->num_filhos; i++) {
        liberarAST(raiz->filhos[i]);
    }

    if (raiz->filhos) {
        free(raiz->filhos);
        raiz->filhos = NULL;
    }

    if (raiz->tipo_no == NO_IDENTIFICADOR) {
        free(raiz->valor.valor_str);
    }
    free(raiz);
}
