#include "codegen.h"

/*
 * Stubs fracos para controle de fluxo e repeticao (Modulo de Pedro Moretti).
 * Quando a implementacao definitiva for adicionada, estas funcoes serao sobrescritas.
 */
__attribute__((weak)) void emitirIf(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}

__attribute__((weak)) void emitirWhile(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}

__attribute__((weak)) void emitirDoWhile(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}

__attribute__((weak)) void emitirFor(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}

__attribute__((weak)) void emitirSwitch(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}

__attribute__((weak)) void emitirComandoFluxo(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}
