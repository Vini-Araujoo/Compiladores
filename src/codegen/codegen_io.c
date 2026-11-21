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

static TipoDado tipoDaExpressao(NoAST *no);

static TipoDado promover(TipoDado a, TipoDado b) {
    if (a == TIPO_DOUBLE || b == TIPO_DOUBLE)
    {
        return TIPO_DOUBLE;
    }
    if (a == TIPO_FLOAT || b == TIPO_FLOAT)
    {
        return TIPO_FLOAT;
    }
    if (a == TIPO_LONG || b == TIPO_LONG)
    {
        return TIPO_LONG;
    }   
    return TIPO_INT;   
}

static TipoDado tipoOpUnaria(NoAST *no)
{
    if (no->num_filhos < 1)
    {
        return TIPO_INDEFINIDO;
    }
    if (strcmp(no->valor.operador, "!") == 0)
    {
        return TIPO_BOOLEAN;
    }
    return tipoDaExpressao(no->filhos[0]);
}

static TipoDado tipoOpBinaria(NoAST *no)
{
    if (no->num_filhos < 2)
    {
        return TIPO_INDEFINIDO;
    }
    const char *op = no->valor.operador;
    if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0 ||
        strcmp(op, "<") == 0  || strcmp(op, ">") == 0  ||
        strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0 ||
        strcmp(op, "==") == 0 || strcmp(op, "!=") == 0)
    {
        return TIPO_BOOLEAN;
    }

    TipoDado esq = tipoDaExpressao(no->filhos[0]);
    TipoDado dir = tipoDaExpressao(no->filhos[1]);

    if (esq == TIPO_INDEFINIDO || dir == TIPO_INDEFINIDO)
    {
        return TIPO_INDEFINIDO;
    }

    if (strcmp(op, "+") == 0 && (esq == TIPO_STRING || dir == TIPO_STRING))
    {
        return TIPO_STRING;
    }
    return promover(esq, dir);
}

static TipoDado tipoDaExpressao(NoAST *no)
{
    if (no == NULL)
    {
        return TIPO_INDEFINIDO;
    }

    switch (no->tipo_no)
    {
        case NO_LITERAL_INT:
            return TIPO_INT;

        case NO_LITERAL_DOUBLE:
            return TIPO_DOUBLE;

        case NO_LITERAL_BOOL:
            return TIPO_BOOLEAN;

        case NO_LITERAL_STRING:
            return TIPO_STRING;

        case NO_IDENTIFICADOR:
            if (no->simbolo != NULL)
            {
                return no->simbolo->tipo;
            }
            return no->tipo_dado;

        case NO_EXPRESSAO:
            if (no->num_filhos == 1)
            {
                return tipoDaExpressao(no->filhos[0]);
            }
            return TIPO_INDEFINIDO;

        case NO_OP_UNARIA:
            return tipoOpUnaria(no);

        case NO_OP_BINARIA:
            return tipoOpBinaria(no);

        default:
            return TIPO_INDEFINIDO;
    }
}

static void emitirSaida(NoAST *no, FILE *saida, int nivel, int quebraLinha)
{
    if (no == NULL)
    {
        fprintf(stderr, "Erro de geracao: no de saida nulo.\n");
        return;
    }
    if (no->num_filhos == 0)
    {
        if (quebraLinha)
        {
            emitirIndentacao(saida, nivel);
            fprintf(saida, "printf(\"\\n\");\n");
        }
        else
        {
            fprintf(stderr, "Erro de geracao na linha %d: print sem argumento.\n", no->linha);
        }
        return;
    }
    NoAST *expr = no->filhos[0];
    TipoDado tipo = tipoDaExpressao(expr);
    const char *mascara = mascaraPrintf(tipo);

    if (mascara == NULL)
    {
        fprintf(stderr, "Erro de geracao na linha %d: tipo da expressao indefinido ou nao suportado.\n", no->linha);
        return;
    }

    emitirIndentacao(saida, nivel);
    fprintf(saida, "%s", "printf(\"");
    fprintf(saida, "%s%s", mascara, quebraLinha ? "\\n" : "");
    fprintf(saida, "%s", "\", ");
    if (tipo == TIPO_BOOLEAN)
    {
        fprintf(saida, "(");
        emitirExpressao(expr, saida);
        fprintf(saida, ") ? \"true\" : \"false\"");
    }
    else
    {
        emitirExpressao(expr, saida);
    }
    fprintf(saida, "%s", ");\n");
}

void emitirPrintln(NoAST *no, FILE *saida, int nivel)
{
    emitirSaida(no, saida, nivel, 1);
}

void emitirPrint(NoAST *no, FILE *saida, int nivel)
{
    emitirSaida(no, saida, nivel, 0);
}
