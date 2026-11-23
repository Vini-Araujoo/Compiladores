#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "tabela.h"
#include "semantica.h"
#include "codegen.h"

extern int yyparse(void);
extern FILE *yyin;
extern NoAST *arvore;

int main(int argc, char **argv)
{
    // apontando pro arquivo de entrada
    FILE *entrada = stdin;

    // arq de saida
    FILE *saida = stdout;

    // abrimos o arquivo .java
    if (argc > 1)
    {
        entrada = fopen(argv[1], "r");
        if (!entrada)
        {
            perror("Erro ao abrir arquivo");
            return 1;
        }

        // o flex aponta pro arquivo de entrada
        yyin = entrada;
    }

    // Análise Léxica e Sintática
    // yyparse() devolve 0 em sucesso e 1 em erro de sintaxe (bison)
    if (yyparse() != 0 || !arvore)
    {
        fprintf(stderr, "Erro sintatico ao analisar o Java.\n");
        if (entrada != stdin)
            fclose(entrada);
        return 1;
    }

    // Análise Semântica (valida tipos, variáveis, escopos)
    if (!verificarEscopos(arvore))
    {
        fprintf(stderr, "Erro semantico encontrado.\n");
        liberarAST(arvore);
        liberarTabelaSimbolos();
        if (entrada != stdin)
            fclose(entrada);
        return 1;
    }

    //  gencode (chama a func orquestradora)
    gerarCodigoC(arvore, saida);

    // free all
    liberarAST(arvore);
    liberarTabelaSimbolos();
    if (entrada != stdin)
        fclose(entrada);
    return 0;
}