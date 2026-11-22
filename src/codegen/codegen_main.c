#include "codegen.h"
#include <stdarg.h> //serve pra tratar a quantidade de parametros a mais (como o impl no emitirLinha) (chamado de varargs)

void emitirIndentacao(FILE *saida, int nivel)
{
    for (int i = 0; i < nivel; i++)
    {
        fprintf(saida, "\t");
    }
}

// Em vez de void emitirLinha(...
void emitirCabecalho(FILE *saida)
{
    fprintf(saida, "#include <stdio.h>\n");
    fprintf(saida, "#include <stdlib.h>\n");
    fprintf(saida, "#include <stdbool.h>\n");
    fprintf(saida, "#include <string.h>\n\n");
}
void emitirLinha(FILE *saida, int nivel, const char *format, ...)
{
    emitirIndentacao(saida, nivel);

    va_list args;

    va_start(args, format);        // começa a ler tudo que vem depois do format(apartir do ...) salva no struct
    vfprintf(saida, format, args); // le e preenche os tipos
    va_end(args);

    fprintf(saida, "\n");
}

void emitirMetodo(NoAST *no, FILE *saida, int nivel)
{
    if (no != NULL && no->tipo_no == NO_METODO)
    {

        if (strcmp(no->valor.valor_str, "main") == 0)
        {
            emitirLinha(saida, nivel, "int main(int argc, char *argv[]){");

            NoAST *corpo = no->filhos[1];
            if (corpo)
            {
                for (int i = 0; i < corpo->num_filhos; i++)
                {
                    NoAST *comando = corpo->filhos[i];

                    emitirInstrucao(comando, saida, nivel + 1);
                }
            }
            emitirLinha(saida, nivel + 1, "return 0;");
            emitirLinha(saida, nivel, "}\n");
        }
        else
        {
            const char *retornoMetodo = traduzirTipoC(no->tipo_dado);
            const char *nomeMetodo = no->valor.valor_str;

            emitirIndentacao(saida, nivel);
            fprintf(saida, "%s %s(", retornoMetodo, nomeMetodo);

            NoAST *parametros = no->filhos[0]; // o filho 0 sao os param do metodo

            if (parametros != NULL)
            {
                // percorremos os filhos dos parametros identificando tipo e nome e logo printamos
                for (int i = 0; i < parametros->num_filhos; i++)
                {
                    NoAST *p = parametros->filhos[i];
                    const char *tipoParam = traduzirTipoC(p->tipo_dado);
                    const char *nomeParam = p->valor.valor_str;

                    if (i > 0)
                    {
                        fprintf(saida, ", ");
                    }
                    fprintf(saida, "%s %s", tipoParam, nomeParam);
                }
            }
            fprintf(saida, ") {\n"); //) e { do metodo

            // termina de imprimir o resto

            NoAST *corpo = no->filhos[1];
            if (corpo)
            {
                for (int i = 0; i < corpo->num_filhos; i++)
                {
                    NoAST *comando = corpo->filhos[i];

                    emitirInstrucao(comando, saida, nivel + 1);
                }
            }

            emitirLinha(saida, nivel, "}\n");
        }
    }
}

void emitirChamadaMetodo(NoAST *no, FILE *saida)
{
    (void)no;
    (void)saida;
}

__attribute__((weak)) void emitirInstrucao(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}

void gerarCodigoC(NoAST *raiz, FILE *saida)
{
    (void)raiz;
    (void)saida;
}

/*
Se NÃO for "main":
Descobre o tipo de retorno em C (usando o traduzirTipoC(no->tipo_dado) que o Vinícius implementou)
Emite o tipo, o nome da função e abre parênteses (
Percorre os parâmetros em no->filhos[0] emitindo tipo nome, tipo nome
Abre a chave {
Percorre as instruções do corpo (no->filhos[1])
Fecha com }
*/