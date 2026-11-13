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