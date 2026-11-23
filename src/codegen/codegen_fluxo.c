#include "codegen.h"

/*
 * Stubs fracos para controle de fluxo e repeticao (Modulo de Pedro Moretti).
 * Quando a implementacao definitiva for adicionada, estas funcoes serao sobrescritas.
 */

static void emitirIfInterno(NoAST *no, FILE *saida, int nivel,
                            bool isInline){
    if(isInline){
        fprintf(saida, " ");
    } else {
        emitirIndentacao(saida, nivel);
    }

    fprintf(saida, "if (");
    emitirExpressao(no->filhos[0], saida);
    fprintf(saida, ") {\n");

    if(no->filhos[1]->tipo_no == NO_BLOCO){
        NoAST *no_comando_if = no->filhos[1];
        for(int x = 0; x < no_comando_if->num_filhos; x++){
            emitirInstrucao(no_comando_if->filhos[x], saida, nivel+1);
        }
    } else {
        emitirInstrucao(no->filhos[1], saida, nivel + 1);
    }
    emitirIndentacao(saida, nivel);
    fprintf(saida, "}");
    if(no->num_filhos > 2 && no->filhos[2]){
        NoAST *no_comando_else = no->filhos[2];
        fprintf(saida, " else");
        if(no_comando_else->tipo_no == NO_IF){
            emitirIfInterno(no_comando_else, saida, nivel, true);
        } else {
            fprintf(saida, " {\n");
            if(no_comando_else->tipo_no == NO_BLOCO){
                for(int x = 0; x < no_comando_else->num_filhos; x++){
                    emitirInstrucao(no_comando_else->filhos[x], saida, nivel+1);
                }
            } else {
                emitirInstrucao(no_comando_else, saida, nivel + 1);
            }
            emitirIndentacao(saida, nivel);
            fprintf(saida, "}");
            fprintf(saida, "\n");
        }
    } else {
        fprintf(saida, "\n");
    }
}

void emitirIf(NoAST *no, FILE *saida, int nivel)
{
    emitirIfInterno(no, saida, nivel, false);
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
