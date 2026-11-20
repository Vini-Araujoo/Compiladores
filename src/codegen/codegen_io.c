#include "codegen.h"

static const char *mascaraPrintf(TipoDado tipo) {
    switch (tipo) {
    case TIPO_INT:
    case TIPO_SHORT:
    case TIPO_BYTE:
        return "%d";

    case TIPO_LONG:
        return "%lld";

    case TIPO_FLOAT:
    case TIPO_DOUBLE:
        return "%lf";

    case TIPO_STRING:
    case TIPO_BOOLEAN:
        return "%s";

    case TIPO_CHAR:
        return "%c";

    default:
        return NULL;
    }
}

static const char * mascaraScanf(const char * metodo) {
    if (metodo == NULL)
    {
        return NULL;
    }
    if (!strcmp(metodo, "nextInt") || !strcmp(metodo, "nextBoolean"))
    {
        return "%d";
    }
    if (!strcmp(metodo, "nextDouble"))
    {
        return "%lf";
    }
    if (!strcmp(metodo, "nextFloat"))
    {
        return "%f";
    }
    if (!strcmp(metodo, "nextLong"))
    {
        return "%lld";
    }

    if (!strcmp(metodo, "nextShort"))
    {
        return "%hd";
    }
    if (!strcmp(metodo, "nextByte"))
    {
        return "%hhd";
    }
    if (!strcmp(metodo, "next"))
    {
        return "%s";
    }
    if (!strcmp(metodo, "nextLine"))
    {
        return " %[^\\n]";
    }
    return NULL;
}

static int precisaEndereco(const char * metodo) {
    if (metodo == NULL || !strcmp(metodo, "next") || !strcmp(metodo, "nextLine"))
    {
        return 0;
    }
    return 1;    
}

void emitirScannerLeituraEm(NoAST *no, const char *destino, FILE *saida, int nivel) {

    if (no == NULL || destino == NULL)
    {
        fprintf(stderr, "Erro de geracao: leitura do Scanner sem no ou sem destino\n");
        return;
    }

    const char *metodo = no->valor.valor_str;
    const char *mascara = mascaraScanf(metodo);

    if (mascara == NULL)
    {
        fprintf(stderr, "Erro de geracao na linha %d: metodo do Scanner desconhecido.\n", no->linha);
        return;
    }

    emitirIndentacao(saida, nivel);
    fprintf(saida, "scanf(\"");
    fprintf(saida, "%s\", ", mascara);
    if (precisaEndereco(metodo))
    {
        fprintf(saida, "%s", "&");
    }
    fprintf(saida, "%s);\n", destino);    
}

void emitirScannerLeitura(NoAST *no, FILE *saida, int nivel)
{
    (void)saida;
    (void)nivel;

    if (no == NULL)
    {
        fprintf(stderr, "Erro de geracao: no de leitura do Scanner nulo.\n");
        return;
    }

    fprintf(stderr,
            "Erro de geracao na linha %d: leitura '%s' do Scanner sem atribuicao nao e suportada.\n",
            no->linha, no->valor.valor_str ? no->valor.valor_str : "?");
}

void emitirInstanciacaoScanner(NoAST *no, FILE *saida, int nivel) {

    if (no == NULL)
    {
        fprintf(stderr, "Erro de geracao: no de instanciacao do Scanner nulo.\n");
        return;
    }

    emitirIndentacao(saida, nivel);

    fprintf(saida, "/* Scanner: entrada padrao (stdin) ja aberta */\n");
}

