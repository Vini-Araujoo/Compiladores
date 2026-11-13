#ifndef INFERENCIA_H
#define INFERENCIA_H

#include "ast.h"

/*
 * Infere, de baixo para cima, o tipo de uma expressao e grava o resultado em
 * tipo_dado de cada no intermediario. Devolve TIPO_INDEFINIDO quando o tipo
 * nao pode ser determinado.
 */
TipoDado inferirTipoExpressao(NoAST *expr);

/*
 * Percorre a AST, infere o tipo de cada expressao encontrada e emite em
 * stderr um erro semantico, com a linha, para cada expressao de tipo
 * indeterminado. Devolve a quantidade de erros.
 */
int analisarTiposAST(NoAST *raiz);

#endif