#include <stdio.h>
#include "ast.h"
#include "inferencia.h"

extern int yyparse(void);
extern NoAST *arvore;

int main(void) {
    if (yyparse() != 0) {
        return 1;
    }

    if (analisarTiposAST(arvore) > 0) {
        liberarAST(arvore);
        return 1;
    }

    if (arvore) {
        imprimirASTComTipos(arvore);
    }
    printf("\nAnalise semantica de tipos concluida com sucesso.\n");
    liberarAST(arvore);
    return 0;
}