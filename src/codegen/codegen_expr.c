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

static const char *const OPS_ARITMETICOS[] = {"+", "-", "*", "/", "%", NULL};
static const char *const OPS_RELACIONAIS[] = {"==", "!=", "<", "<=", ">", ">=", NULL};
static const char *const OPS_LOGICOS[] = {"&&", "||", NULL};

static int operadorEm(const char *op, const char *const *lista)
{
    for (int i = 0; lista[i] != NULL; i++)
    {
        if (strcmp(op, lista[i]) == 0)
            return 1;
    }
    return 0;
}

static void emitirBinarioParentizado(NoAST *no, FILE *saida)
{
    if (no->num_filhos != 2 || no->filhos[0] == NULL || no->filhos[1] == NULL ||
        no->valor.operador[0] == '\0')
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

static void emitirOpAritmetica(NoAST *no, FILE *saida)
{
    emitirBinarioParentizado(no, saida);
}

static void emitirOpRelacional(NoAST *no, FILE *saida)
{
    emitirBinarioParentizado(no, saida);
}

static void emitirOpLogico(NoAST *no, FILE *saida)
{
    if (no->tipo_no == NO_OP_UNARIA)
    {
        if (no->num_filhos != 1 || no->filhos[0] == NULL)
        {
            fprintf(stderr, "codegen: linha %d: negacao sem operando\n", no->linha);
            return;
        }
        fputs("(!(", saida);
        emitirExpressao(no->filhos[0], saida);
        fputs("))", saida);
        return;
    }
    emitirBinarioParentizado(no, saida);
}

void emitirOpBinaria(NoAST *no, FILE *saida)
{
    const char *op = no->valor.operador;
    if (operadorEm(op, OPS_ARITMETICOS))
        emitirOpAritmetica(no, saida);
    else if (operadorEm(op, OPS_RELACIONAIS))
        emitirOpRelacional(no, saida);
    else if (operadorEm(op, OPS_LOGICOS))
        emitirOpLogico(no, saida);
    else
        fprintf(stderr, "codegen: linha %d: operador binario desconhecido \"%s\"\n", no->linha, op);
}

static void emitirIncDec(NoAST *operando, const char *op, int posfixo, FILE *saida)
{
    fputc('(', saida);
    if (!posfixo)
        fputs(op, saida);
    emitirExpressao(operando, saida);
    if (posfixo)
        fputs(op, saida);
    fputc(')', saida);
}

void emitirOpUnaria(NoAST *no, FILE *saida)
{
    if (no->num_filhos < 1 || no->filhos[0] == NULL)
    {
        fprintf(stderr, "codegen: linha %d: operacao unaria sem operando\n", no->linha);
        return;
    }
    NoAST *operando = no->filhos[0];

    switch (no->tipo_no)
    {
    case NO_INCREMENTO:
        emitirIncDec(operando, "++", 1, saida);
        return;
    case NO_DECREMENTO:
        emitirIncDec(operando, "--", 1, saida);
        return;
    case NO_INCREMENTO_ANTES:
        emitirIncDec(operando, "++", 0, saida);
        return;
    case NO_DECREMENTO_ANTES:
        emitirIncDec(operando, "--", 0, saida);
        return;
    default:
        break;
    }

    const char *op = no->valor.operador;
    if (strcmp(op, "!") == 0)
    {
        emitirOpLogico(no, saida);
    }
    else if (strcmp(op, "-") == 0 || strcmp(op, "+") == 0)
    {
        fputc('(', saida);
        fputs(op, saida);
        emitirExpressao(operando, saida);
        fputc(')', saida);
    }
    else if (strcmp(op, "++") == 0 || strcmp(op, "--") == 0)
    {
        emitirIncDec(operando, op, no->posfixo, saida);
    }
    else
    {
        fprintf(stderr, "codegen: linha %d: operador unario desconhecido \"%s\"\n", no->linha, op);
    }
}

void emitirExpressao(NoAST *no, FILE *saida)
{
    if (no == NULL)
        return;

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
    case NO_OP_UNARIA:
    case NO_INCREMENTO:
    case NO_DECREMENTO:
    case NO_INCREMENTO_ANTES:
    case NO_DECREMENTO_ANTES:
        emitirOpUnaria(no, saida);
        break;
    case NO_ACESSO_ARRAY:
        emitirAcessoArray(no, saida);
        break;
    case NO_CHAMADA_METODO:
        emitirChamadaMetodo(no, saida);
        break;
    default:
        fprintf(stderr, "codegen: linha %d: expressao nao suportada (tipo_no=%d)\n",
                no->linha, (int)no->tipo_no);
    }
}