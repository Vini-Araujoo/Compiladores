#include "codegen.h"

static void emitirInteiro(long v, FILE *saida)
{
    if (v < 0)
        fprintf(saida, "(%ldLL)", v);
    else if (v > 2147483647L)
        fprintf(saida, "%ldLL", v);
    else
        fprintf(saida, "%ld", v);
}

static void emitirReal(double v, int ehFloat, FILE *saida)
{
    char buf[40];
    int maxPrecisao = ehFloat ? 9 : 17;
    for (int precisao = 1; precisao <= maxPrecisao; precisao++)
    {
        snprintf(buf, sizeof buf, "%.*g", precisao, v);
        if (ehFloat ? (strtof(buf, NULL) == (float)v) : (strtod(buf, NULL) == v))
            break;
    }
    if (strpbrk(buf, ".eE") == NULL)
        strcat(buf, ".0");
    if (ehFloat)
        strcat(buf, "f");
    if (buf[0] == '-')
        fprintf(saida, "(%s)", buf);
    else
        fputs(buf, saida);
}

static void emitirChar(long c, FILE *saida)
{
    if (c < 0 || c > 255)
    {
        fprintf(saida, "%ld", c);
        return;
    }
    fputc('\'', saida);
    switch (c)
    {
    case '\n': fputs("\\n", saida);  break;
    case '\t': fputs("\\t", saida);  break;
    case '\r': fputs("\\r", saida);  break;
    case '\\': fputs("\\\\", saida); break;
    case '\'': fputs("\\'", saida);  break;
    case '\0': fputs("\\0", saida);  break;
    default:
        if (c >= 0x20 && c < 0x7F)
            fputc((int)c, saida);
        else
            fprintf(saida, "\\%03lo", c);
    }
    fputc('\'', saida);
}

void emitirLiteral(NoAST *no, FILE *saida)
{
    switch (no->tipo_no)
    {
    case NO_LITERAL_INT:
        emitirInteiro(no->valor.valor_int, saida);
        break;
    case NO_LITERAL_FLOAT:
        emitirReal(no->valor.valor_double, 1, saida);
        break;
    case NO_LITERAL_DOUBLE:
        emitirReal(no->valor.valor_double, 0, saida);
        break;
    case NO_LITERAL_CHAR:
        emitirChar(no->valor.valor_int, saida);
        break;
    case NO_LITERAL_BOOL:
        fputs(no->valor.valor_int ? "true" : "false", saida);
        break;
    case NO_LITERAL_STRING:
        fprintf(saida, "\"%s\"", no->valor.valor_str ? no->valor.valor_str : "");
        break;
    default:
        fprintf(stderr, "codegen: linha %d: literal nao suportado (tipo_no=%d)\n",
                no->linha, (int)no->tipo_no);
    }
}

void emitirIdentificador(NoAST *no, FILE *saida)
{
    if (no->valor.valor_str == NULL)
    {
        fprintf(stderr, "codegen: linha %d: identificador sem nome\n", no->linha);
        return;
    }
    fputs(no->valor.valor_str, saida);
}

void emitirOpBinaria(NoAST *no, FILE *saida)
{
    if (no->num_filhos != 2 || no->valor.operador[0] == '\0')
    {
        fprintf(stderr, "codegen: linha %d: operacao binaria malformada\n", no->linha);
        return;
    }
    fputc('(', saida);
    emitirExpressao(no->filhos[0], saida);
    fprintf(saida, " %s ", no->valor.operador);
    emitirExpressao(no->filhos[1], saida);
    fputc(')', saida);
}

void emitirExpressao(NoAST *no, FILE *saida)
{
    if (no == NULL)
    {
        fprintf(stderr, "codegen: expressao nula\n");
        return;
    }
    switch (no->tipo_no)
    {
    case NO_LITERAL_INT:
    case NO_LITERAL_FLOAT:
    case NO_LITERAL_DOUBLE:
    case NO_LITERAL_CHAR:
    case NO_LITERAL_BOOL:
    case NO_LITERAL_STRING:
        emitirLiteral(no, saida);
        break;
    case NO_IDENTIFICADOR:
        emitirIdentificador(no, saida);
        break;
    case NO_OP_BINARIA:
        emitirOpBinaria(no, saida);
        break;
    default:
        fprintf(stderr, "codegen: linha %d: expressao nao suportada (tipo_no=%d)\n",
                no->linha, (int)no->tipo_no);
    }
}