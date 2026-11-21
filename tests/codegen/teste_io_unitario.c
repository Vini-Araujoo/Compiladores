#include "codegen.h"

static int total = 0;
static int falhas = 0;

/* ---------- stub fraco de emitirExpressao ---------- */
__attribute__((weak)) void emitirExpressao(NoAST *no, FILE *saida)
{
    switch (no->tipo_no)
    {
        case NO_IDENTIFICADOR:
            fprintf(saida, "%s", no->valor.valor_str);
            break;
        case NO_LITERAL_INT:
            fprintf(saida, "%ld", no->valor.valor_int);
            break;
        case NO_LITERAL_STRING:
            fprintf(saida, "\"%s\"", no->valor.valor_str);
            break;
        case NO_OP_BINARIA:
            emitirExpressao(no->filhos[0], saida);
            fprintf(saida, " %s ", no->valor.operador);
            emitirExpressao(no->filhos[1], saida);
            break;
        default:
            fprintf(saida, "?");
            break;
    }
}

/* ---------- helpers para montar a AST ---------- */
static NoAST *id(const char *nome, TipoDado tipo)
{
    NoAST *n = criarNo(NO_IDENTIFICADOR, 1);
    n->valor.valor_str = (char *)nome;
    n->tipo_dado = tipo;
    return n;
}

static NoAST *litInt(long v)
{
    NoAST *n = criarNo(NO_LITERAL_INT, 1);
    n->valor.valor_int = v;
    return n;
}

static NoAST *litStr(const char *s)
{
    NoAST *n = criarNo(NO_LITERAL_STRING, 1);
    n->valor.valor_str = (char *)s;
    return n;
}

static NoAST *bin(const char *op, NoAST *a, NoAST *b)
{
    NoAST *n = criarNo(NO_OP_BINARIA, 1);
    strncpy(n->valor.operador, op, 3);
    n->valor.operador[3] = '\0';
    adicionarFilho(n, a);
    adicionarFilho(n, b);
    return n;
}

static NoAST *comando(TipoNoAST tipo, NoAST *arg)
{
    NoAST *n = criarNo(tipo, 1);
    if (arg != NULL)
    {
        adicionarFilho(n, arg);
    }
    return n;
}

static NoAST *leitura(const char *metodo)
{
    NoAST *n = criarNo(NO_LEITURA_SCANNER, 1);
    n->valor.valor_str = (char *)metodo;
    return n;
}

/* ---------- captura da saida ---------- */
static FILE *abrir(char **buf, size_t *tam)
{
    *buf = NULL;
    *tam = 0;
    return open_memstream(buf, tam);
}

static void conferir(const char *nome, FILE *f, char **buf, const char *esperado)
{
    fclose(f);
    total++;
    const char *obtido = (*buf != NULL) ? *buf : "";

    if (strcmp(obtido, esperado) == 0)
    {
        printf("  [PASS] %s\n", nome);
    }
    else
    {
        printf("  [FAIL] %s\n         esperado: \"%s\"\n         obtido:   \"%s\"\n",
               nome, esperado, obtido);
        falhas++;
    }
    free(*buf);
}

static void testarSaida(const char *nome, TipoNoAST tipo, NoAST *arg, const char *esperado)
{
    char *buf;
    size_t tam;
    FILE *f = abrir(&buf, &tam);
    NoAST *no = comando(tipo, arg);

    if (tipo == NO_PRINTLN)
    {
        emitirPrintln(no, f, 1);
    }
    else
    {
        emitirPrint(no, f, 1);
    }
    conferir(nome, f, &buf, esperado);
}

static void testarLeituraEm(const char *metodo, const char *destino, const char *esperado)
{
    char nome[64];
    char *buf;
    size_t tam;
    FILE *f = abrir(&buf, &tam);

    snprintf(nome, sizeof nome, "scanner %s", metodo);
    emitirScannerLeituraEm(leitura(metodo), destino, f, 1);
    conferir(nome, f, &buf, esperado);
}

int main(void)
{
    printf("== Scanner: scanf por metodo ==\n");
    testarLeituraEm("nextInt", "x", "\tscanf(\"%d\", &x);\n");
    testarLeituraEm("nextDouble", "d", "\tscanf(\"%lf\", &d);\n");
    testarLeituraEm("nextFloat", "f", "\tscanf(\"%f\", &f);\n");
    testarLeituraEm("nextLong", "l", "\tscanf(\"%lld\", &l);\n");
    testarLeituraEm("nextShort", "s", "\tscanf(\"%hd\", &s);\n");
    testarLeituraEm("nextByte", "b", "\tscanf(\"%hhd\", &b);\n");
    testarLeituraEm("nextBoolean", "ok", "\tscanf(\"%d\", &ok);\n");
    testarLeituraEm("next", "palavra", "\tscanf(\"%s\", palavra);\n");
    testarLeituraEm("nextLine", "linha", "\tscanf(\" %[^\\n]\", linha);\n");
    testarLeituraEm("nextXyz", "x", "");

    printf("== Scanner: instanciacao e leitura solta ==\n");
    {
        char *buf;
        size_t tam;
        FILE *f = abrir(&buf, &tam);
        emitirInstanciacaoScanner(criarNo(NO_INSTANCIACAO_SCANNER, 1), f, 1);
        conferir("instanciacao do Scanner", f, &buf,
                 "\t/* Scanner: entrada padrao (stdin) ja aberta */\n");
    }
    {
        char *buf;
        size_t tam;
        FILE *f = abrir(&buf, &tam);
        emitirScannerLeitura(leitura("nextInt"), f, 1);
        conferir("leitura sem destino gera erro e nao emite", f, &buf, "");
    }

    printf("== println/print: mascara por tipo ==\n");
    testarSaida("println int", NO_PRINTLN, id("x", TIPO_INT), "\tprintf(\"%d\\n\", x);\n");
    testarSaida("println short", NO_PRINTLN, id("x", TIPO_SHORT), "\tprintf(\"%d\\n\", x);\n");
    testarSaida("println byte", NO_PRINTLN, id("x", TIPO_BYTE), "\tprintf(\"%d\\n\", x);\n");
    testarSaida("println long", NO_PRINTLN, id("x", TIPO_LONG), "\tprintf(\"%lld\\n\", x);\n");
    testarSaida("println float", NO_PRINTLN, id("x", TIPO_FLOAT), "\tprintf(\"%lf\\n\", x);\n");
    testarSaida("println double", NO_PRINTLN, id("x", TIPO_DOUBLE), "\tprintf(\"%lf\\n\", x);\n");
    testarSaida("println string", NO_PRINTLN, id("s", TIPO_STRING), "\tprintf(\"%s\\n\", s);\n");
    testarSaida("println char", NO_PRINTLN, id("c", TIPO_CHAR), "\tprintf(\"%c\\n\", c);\n");
    testarSaida("println boolean", NO_PRINTLN, id("b", TIPO_BOOLEAN),
                "\tprintf(\"%s\\n\", (b) ? \"true\" : \"false\");\n");

    printf("== println/print: literais e expressoes ==\n");
    testarSaida("println literal string", NO_PRINTLN, litStr("Ola"), "\tprintf(\"%s\\n\", \"Ola\");\n");
    testarSaida("println literal int", NO_PRINTLN, litInt(100), "\tprintf(\"%d\\n\", 100);\n");
    testarSaida("println i + 1 (int)", NO_PRINTLN, bin("+", id("i", TIPO_INT), litInt(1)),
                "\tprintf(\"%d\\n\", i + 1);\n");
    testarSaida("println i + d (double)", NO_PRINTLN, bin("+", id("i", TIPO_INT), id("d", TIPO_DOUBLE)),
                "\tprintf(\"%lf\\n\", i + d);\n");
    testarSaida("println i > 5 (boolean)", NO_PRINTLN, bin(">", id("i", TIPO_INT), litInt(5)),
                "\tprintf(\"%s\\n\", (i > 5) ? \"true\" : \"false\");\n");

    printf("== println/print: variacoes ==\n");
    testarSaida("println sem argumento", NO_PRINTLN, NULL, "\tprintf(\"\\n\");\n");
    testarSaida("print int (sem \\n)", NO_PRINT, id("x", TIPO_INT), "\tprintf(\"%d\", x);\n");
    testarSaida("print string (sem \\n)", NO_PRINT, id("s", TIPO_STRING), "\tprintf(\"%s\", s);\n");
    testarSaida("print boolean (sem \\n)", NO_PRINT, id("b", TIPO_BOOLEAN),
                "\tprintf(\"%s\", (b) ? \"true\" : \"false\");\n");
    testarSaida("print sem argumento gera erro", NO_PRINT, NULL, "");
    testarSaida("println de tipo indefinido gera erro", NO_PRINTLN, id("v", TIPO_VOID), "");

    printf("\n%d testes, %d falha(s)\n", total, falhas);
    return falhas == 0 ? 0 : 1;
}
