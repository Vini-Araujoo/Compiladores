#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"

static int total = 0;
static int falhas = 0;

/* Contexto para o stub de emitirScannerLeitura verificar a delegacao */
static const char *ultimo_destino_scanner = "x";
static int chamadas_scanner_leitura = 0;
static int ultimo_nivel_scanner = -1;
static char ultimo_metodo_scanner[64] = "";

/* Stub fraco de emitirExpressao (modulo de Pedro Augusto) */
__attribute__((weak)) void emitirExpressao(NoAST *no, FILE *saida)
{
    if (no == NULL)
    {
        return;
    }
    switch (no->tipo_no)
    {
        case NO_IDENTIFICADOR:
            fprintf(saida, "%s", no->valor.valor_str ? no->valor.valor_str : "");
            break;
        case NO_LITERAL_INT:
            fprintf(saida, "%ld", no->valor.valor_int);
            break;
        case NO_LITERAL_DOUBLE:
        case NO_LITERAL_FLOAT:
            fprintf(saida, "%g", no->valor.valor_double);
            break;
        case NO_LITERAL_BOOL:
            fprintf(saida, "%s", no->valor.valor_int ? "true" : "false");
            break;
        case NO_LITERAL_CHAR:
            fprintf(saida, "'%c'", (char)no->valor.valor_int);
            break;
        case NO_LITERAL_STRING:
            fprintf(saida, "\"%s\"", no->valor.valor_str ? no->valor.valor_str : "");
            break;
        case NO_OP_BINARIA:
            emitirExpressao(no->filhos[0], saida);
            fprintf(saida, " %s ", no->valor.operador);
            emitirExpressao(no->filhos[1], saida);
            break;
        default:
            fprintf(saida, "/*expr*/");
            break;
    }
}

/* Stub fraco de emitirScannerLeitura (modulo de Natan Franca) */
__attribute__((weak)) void emitirScannerLeitura(NoAST *no, FILE *saida, int nivel)
{
    chamadas_scanner_leitura++;
    ultimo_nivel_scanner = nivel;
    if (no == NULL || no->valor.valor_str == NULL)
    {
        return;
    }

    strncpy(ultimo_metodo_scanner, no->valor.valor_str, sizeof(ultimo_metodo_scanner) - 1);
    ultimo_metodo_scanner[sizeof(ultimo_metodo_scanner) - 1] = '\0';

    const char *metodo = no->valor.valor_str;
    emitirIndentacao(saida, nivel);
    if (strcmp(metodo, "nextInt") == 0)
    {
        fprintf(saida, "scanf(\"%%d\", &%s);\n", ultimo_destino_scanner);
    }
    else if (strcmp(metodo, "nextDouble") == 0)
    {
        fprintf(saida, "scanf(\"%%lf\", &%s);\n", ultimo_destino_scanner);
    }
    else if (strcmp(metodo, "nextLine") == 0)
    {
        fprintf(saida, "scanf(\" %%[^\\n]\", %s);\n", ultimo_destino_scanner);
    }
    else
    {
        fprintf(saida, "scanf(\"%%s\", %s);\n", ultimo_destino_scanner);
    }
}

static char *lerArquivoTemporario(FILE *f)
{
    fflush(f);
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    if (tam < 0)
    {
        tam = 0;
    }
    rewind(f);

    char *buf = (char *)malloc((size_t)tam + 1);
    assert(buf != NULL);
    size_t lidos = fread(buf, 1, (size_t)tam, f);
    buf[lidos] = '\0';
    return buf;
}

static void conferirTexto(const char *nome_teste, const char *obtido, const char *esperado)
{
    total++;
    if (obtido != NULL && strcmp(obtido, esperado) == 0)
    {
        printf("  [PASS] %s\n", nome_teste);
    }
    else
    {
        printf("  [FAIL] %s\n         esperado: \"%s\"\n         obtido:   \"%s\"\n",
               nome_teste, esperado, obtido ? obtido : "(null)");
        falhas++;
    }
}

static void conferirTipo(const char *nome_teste, TipoDado tipo, const char *esperado)
{
    const char *obtido = traduzirTipoC(tipo);
    total++;
    if ((esperado == NULL && obtido == NULL) ||
        (esperado != NULL && obtido != NULL && strcmp(obtido, esperado) == 0))
    {
        printf("  [PASS] %s\n", nome_teste);
    }
    else
    {
        printf("  [FAIL] %s\n         esperado: %s\n         obtido:   %s\n",
               nome_teste, esperado ? esperado : "NULL", obtido ? obtido : "NULL");
        falhas++;
    }
}

static NoAST *criarDecl(TipoDado tipo, const char *nome, int eh_final, NoAST *init)
{
    NoAST *no = criarNo(NO_DECL_VARIAVEL, 1);
    no->tipo_dado = tipo;
    no->valor.valor_str = strdup(nome);
    no->mods.eh_final = eh_final;
    if (init != NULL)
    {
        adicionarFilho(no, init);
    }
    return no;
}

static NoAST *criarLitInt(long v)
{
    NoAST *no = criarNo(NO_LITERAL_INT, 1);
    no->tipo_dado = TIPO_INT;
    no->valor.valor_int = v;
    return no;
}

static NoAST *criarLitDouble(double v)
{
    NoAST *no = criarNo(NO_LITERAL_DOUBLE, 1);
    no->tipo_dado = TIPO_DOUBLE;
    no->valor.valor_double = v;
    return no;
}

static NoAST *criarLitBool(int v)
{
    NoAST *no = criarNo(NO_LITERAL_BOOL, 1);
    no->tipo_dado = TIPO_BOOLEAN;
    no->valor.valor_int = v;
    return no;
}

static NoAST *criarLitChar(char c)
{
    NoAST *no = criarNo(NO_LITERAL_CHAR, 1);
    no->tipo_dado = TIPO_CHAR;
    no->valor.valor_int = (long)c;
    return no;
}

static NoAST *criarLitStr(const char *s)
{
    NoAST *no = criarNo(NO_LITERAL_STRING, 1);
    no->tipo_dado = TIPO_STRING;
    no->valor.valor_str = strdup(s);
    return no;
}

static NoAST *criarLeituraScanner(const char *var_scanner, const char *metodo)
{
    NoAST *id = criarNo(NO_IDENTIFICADOR, 1);
    id->tipo_dado = TIPO_SCANNER;
    id->valor.valor_str = strdup(var_scanner);

    NoAST *leitura = criarNo(NO_LEITURA_SCANNER, 1);
    leitura->valor.valor_str = strdup(metodo);
    adicionarFilho(leitura, id);
    return leitura;
}

static void testarDeclaracao(const char *nome_teste, NoAST *no, int nivel, const char *esperado)
{
    FILE *f = tmpfile();
    assert(f != NULL);
    emitirDeclaracaoVariavel(no, f, nivel);
    char *obtido = lerArquivoTemporario(f);
    fclose(f);

    conferirTexto(nome_teste, obtido, esperado);
    free(obtido);
    liberarAST(no);
}

int main(void)
{
    printf("== Teste 1: Mapeamento de Tipos (traduzirTipoC) ==\n");
    conferirTipo("TIPO_BYTE -> char", TIPO_BYTE, "char");
    conferirTipo("TIPO_SHORT -> short", TIPO_SHORT, "short");
    conferirTipo("TIPO_INT -> int", TIPO_INT, "int");
    conferirTipo("TIPO_LONG -> long long", TIPO_LONG, "long long");
    conferirTipo("TIPO_FLOAT -> float", TIPO_FLOAT, "float");
    conferirTipo("TIPO_DOUBLE -> double", TIPO_DOUBLE, "double");
    conferirTipo("TIPO_CHAR -> char", TIPO_CHAR, "char");
    conferirTipo("TIPO_BOOLEAN -> bool", TIPO_BOOLEAN, "bool");
    conferirTipo("TIPO_STRING -> char *", TIPO_STRING, "char *");
    conferirTipo("TIPO_VOID -> void", TIPO_VOID, "void");
    conferirTipo("TIPO_SCANNER -> NULL", TIPO_SCANNER, NULL);
    conferirTipo("TIPO_ARRAY -> NULL", TIPO_ARRAY, NULL);
    conferirTipo("TIPO_INDEFINIDO -> NULL", TIPO_INDEFINIDO, NULL);

    printf("\n== Teste 2: Declaracoes de Todos os Tipos Primitivos, Boolean e String ==\n");
    testarDeclaracao("byte b;", criarDecl(TIPO_BYTE, "b", 0, NULL), 1, "\tchar b;\n");
    testarDeclaracao("short s;", criarDecl(TIPO_SHORT, "s", 0, NULL), 1, "\tshort s;\n");
    testarDeclaracao("int x = 10;", criarDecl(TIPO_INT, "x", 0, criarLitInt(10)), 1, "\tint x = 10;\n");
    testarDeclaracao("long l = 1000;", criarDecl(TIPO_LONG, "l", 0, criarLitInt(1000)), 1, "\tlong long l = 1000;\n");
    testarDeclaracao("float f = 1.5;", criarDecl(TIPO_FLOAT, "f", 0, criarLitDouble(1.5)), 1, "\tfloat f = 1.5;\n");
    testarDeclaracao("double d = 3.14;", criarDecl(TIPO_DOUBLE, "d", 0, criarLitDouble(3.14)), 2, "\t\tdouble d = 3.14;\n");
    testarDeclaracao("char c = 'A';", criarDecl(TIPO_CHAR, "c", 0, criarLitChar('A')), 1, "\tchar c = 'A';\n");
    testarDeclaracao("boolean ok = true;", criarDecl(TIPO_BOOLEAN, "ok", 0, criarLitBool(1)), 1, "\tbool ok = true;\n");
    testarDeclaracao("String msg = \"Ola\";", criarDecl(TIPO_STRING, "msg", 0, criarLitStr("Ola")), 1, "\tchar *msg = \"Ola\";\n");
    testarDeclaracao("declaracao nivel 0", criarDecl(TIPO_INT, "global", 0, criarLitInt(0)), 0, "int global = 0;\n");

    printf("\n== Teste 3: Modificador final -> const ==\n");
    testarDeclaracao("final int MAX = 100; (via mods.eh_final)",
                     criarDecl(TIPO_INT, "MAX", 1, criarLitInt(100)), 1,
                     "\tconst int MAX = 100;\n");
    testarDeclaracao("final String TITULO = \"Java\";",
                     criarDecl(TIPO_STRING, "TITULO", 1, criarLitStr("Java")), 1,
                     "\tconst char *TITULO = \"Java\";\n");

    {
        /* Teste de deteccao de final via ponteiro simbolo->eh_final */
        Simbolo simb;
        memset(&simb, 0, sizeof(simb));
        simb.nome = "PI";
        simb.tipo = TIPO_DOUBLE;
        simb.eh_final = 1;

        NoAST *no = criarDecl(TIPO_DOUBLE, "PI", 0, criarLitDouble(3.14));
        no->simbolo = &simb;
        testarDeclaracao("final double PI = 3.14; (via simbolo->eh_final)",
                         no, 1, "\tconst double PI = 3.14;\n");
    }

    printf("\n== Teste 4: Declaracoes Multiplas (Conteiner NO_DECL_VARIAVEL) ==\n");
    {
        NoAST *container = criarNo(NO_DECL_VARIAVEL, 1);
        container->tipo_dado = TIPO_INT;
        adicionarFilho(container, criarDecl(TIPO_INT, "a", 0, criarLitInt(1)));
        adicionarFilho(container, criarDecl(TIPO_INT, "b", 0, NULL));
        adicionarFilho(container, criarDecl(TIPO_INT, "c", 0, criarLitInt(3)));

        testarDeclaracao("conteiner int a = 1, b, c = 3;", container, 1,
                         "\tint a = 1;\n\tint b;\n\tint c = 3;\n");
    }
    {
        /* Conteiner propagando tipo e final do pai para os filhos */
        NoAST *container = criarNo(NO_DECL_VARIAVEL, 1);
        container->tipo_dado = TIPO_INT;
        container->mods.eh_final = 1;
        adicionarFilho(container, criarDecl(TIPO_VOID, "K1", 0, criarLitInt(10)));
        adicionarFilho(container, criarDecl(TIPO_VOID, "K2", 0, criarLitInt(20)));

        testarDeclaracao("conteiner final int K1 = 10, K2 = 20; (propagacao)", container, 1,
                         "\tconst int K1 = 10;\n\tconst int K2 = 20;\n");
    }

    printf("\n== Teste 5: Formato Alternativo com Filho NO_IDENTIFICADOR ==\n");
    {
        NoAST *no = criarNo(NO_DECL_VARIAVEL, 1);
        no->tipo_dado = TIPO_INT;
        NoAST *id = criarNo(NO_IDENTIFICADOR, 1);
        id->tipo_dado = TIPO_INT;
        id->valor.valor_str = strdup("idade");
        adicionarFilho(no, id);
        adicionarFilho(no, criarLitInt(25));

        testarDeclaracao("NO_DECL_VARIAVEL com filhos[0]=ID e filhos[1]=10", no, 1,
                         "\tint idade = 25;\n");
    }

    printf("\n== Teste 6: Caso Especial Scanner e Leitura de Scanner ==\n");
    {
        NoAST *inst = criarNo(NO_INSTANCIACAO_SCANNER, 1);
        inst->tipo_dado = TIPO_SCANNER;
        testarDeclaracao("Scanner sc = new Scanner(System.in);",
                         criarDecl(TIPO_SCANNER, "sc", 0, inst), 1,
                         "\t/* Scanner sc */\n");
    }
    {
        chamadas_scanner_leitura = 0;
        ultimo_destino_scanner = "x";
        NoAST *leitura = criarLeituraScanner("sc", "nextInt");
        testarDeclaracao("int x = sc.nextInt();",
                         criarDecl(TIPO_INT, "x", 0, leitura), 1,
                         "\tint x;\n\tscanf(\"%d\", &x);\n");
        total++;
        if (chamadas_scanner_leitura == 1 && ultimo_nivel_scanner == 1 &&
            strcmp(ultimo_metodo_scanner, "nextInt") == 0)
        {
            printf("  [PASS] delegacao para emitirScannerLeitura verificada\n");
        }
        else
        {
            printf("  [FAIL] delegacao para emitirScannerLeitura falhou\n");
            falhas++;
        }
    }
    {
        /* Leitura de Scanner com filho NO_IDENTIFICADOR (filhos[1] = NO_LEITURA_SCANNER) */
        chamadas_scanner_leitura = 0;
        ultimo_destino_scanner = "nota";
        NoAST *no = criarNo(NO_DECL_VARIAVEL, 1);
        no->tipo_dado = TIPO_DOUBLE;
        NoAST *id = criarNo(NO_IDENTIFICADOR, 1);
        id->valor.valor_str = strdup("nota");
        adicionarFilho(no, id);
        adicionarFilho(no, criarLeituraScanner("sc", "nextDouble"));

        testarDeclaracao("double nota = sc.nextDouble(); (formato filhos[0]=ID, filhos[1]=leitura)",
                         no, 2, "\t\tdouble nota;\n\t\tscanf(\"%lf\", &nota);\n");
    }
    {
        /* String lida pelo Scanner usa buffer char nome[256]; */
        chamadas_scanner_leitura = 0;
        ultimo_destino_scanner = "nome";
        NoAST *expr = criarNo(NO_EXPRESSAO, 1);
        adicionarFilho(expr, criarLeituraScanner("sc", "nextLine"));

        testarDeclaracao("String nome = sc.nextLine(); (embrulhado em NO_EXPRESSAO)",
                         criarDecl(TIPO_STRING, "nome", 0, expr), 1,
                         "\tchar nome[256];\n\tscanf(\" %[^\\n]\", nome);\n");
    }

    printf("\n== Teste 7: Expressao Composta na Inicializacao e Casos Defensivos ==\n");
    {
        NoAST *op = criarNo(NO_OP_BINARIA, 1);
        strncpy(op->valor.operador, "+", sizeof(op->valor.operador) - 1);
        NoAST *id1 = criarNo(NO_IDENTIFICADOR, 1);
        id1->valor.valor_str = strdup("MAX");
        NoAST *id2 = criarNo(NO_IDENTIFICADOR, 1);
        id2->valor.valor_str = strdup("MAX");
        adicionarFilho(op, id1);
        adicionarFilho(op, id2);

        testarDeclaracao("int dobro = MAX + MAX;",
                         criarDecl(TIPO_INT, "dobro", 0, op), 1,
                         "\tint dobro = MAX + MAX;\n");
    }
    {
        FILE *f = tmpfile();
        assert(f != NULL);
        emitirDeclaracaoVariavel(NULL, f, 1);
        char *obtido = lerArquivoTemporario(f);
        fclose(f);
        conferirTexto("no nulo nao emite nada e nao falha", obtido, "");
        free(obtido);
    }

    printf("\nResumo teste_decl_unitario: %d passou, %d falhou.\n", total - falhas, falhas);
    return falhas == 0 ? 0 : 1;
}
