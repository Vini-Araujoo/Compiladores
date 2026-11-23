#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "ast.h"

static int total = 0;
static int passou = 0;
static int falhas = 0;

static char *lerArquivoTemporario(FILE *f)
{
    fflush(f);
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    if (tam < 0) tam = 0;
    rewind(f);

    char *buf = (char *)malloc((size_t)tam + 1);
    assert(buf != NULL);
    size_t lidos = fread(buf, 1, (size_t)tam, f);
    buf[lidos] = '\0';
    return buf;
}

static void conferir(const char *nome_teste, const char *obtido, const char *esperado)
{
    total++;
    if (obtido != NULL && strcmp(obtido, esperado) == 0)
    {
        passou++;
        printf("  [PASS] %s\n", nome_teste);
    }
    else
    {
        falhas++;
        printf("  [FAIL] %s\n", nome_teste);
        printf("         esperado: \"%s\"\n", esperado ? esperado : "(null)");
        printf("         obtido:   \"%s\"\n", obtido ? obtido : "(null)");
    }
}

static void teste_main_vazio(void)
{
    NoAST *no_main = criarNo(NO_METODO, 1);
    no_main->valor.valor_str = strdup("main");
    no_main->tipo_dado = TIPO_VOID;

    NoAST *params = criarNo(NO_PARAMETROS, 1);
    NoAST *corpo = criarNo(NO_BLOCO, 1);

    adicionarFilho(no_main, params);
    adicionarFilho(no_main, corpo);

    FILE *f = tmpfile();
    emitirMetodo(no_main, f, 0);

    char *saida = lerArquivoTemporario(f);
    fclose(f);

    const char *esperado =
        "int main(void) {\n"
        "\treturn 0;\n"
        "}\n\n";

    conferir("main vazio", saida, esperado);

    free(saida);
    liberarAST(no_main);
}

static void teste_metodo_sem_parametros(void)
{
    NoAST *no_metodo = criarNo(NO_METODO, 10);
    no_metodo->valor.valor_str = strdup("imprimirLinha");
    no_metodo->tipo_dado = TIPO_VOID;

    NoAST *params = criarNo(NO_PARAMETROS, 10);
    NoAST *corpo = criarNo(NO_BLOCO, 10);

    adicionarFilho(no_metodo, params);
    adicionarFilho(no_metodo, corpo);

    FILE *f = tmpfile();
    emitirMetodo(no_metodo, f, 0);

    char *saida = lerArquivoTemporario(f);
    fclose(f);

    const char *esperado =
        "void imprimirLinha() {\n"
        "}\n\n";

    conferir("metodo void sem parametros", saida, esperado);

    free(saida);
    liberarAST(no_metodo);
}

static void teste_metodo_com_parametros(void)
{
    NoAST *no_metodo = criarNo(NO_METODO, 20);
    no_metodo->valor.valor_str = strdup("somar");
    no_metodo->tipo_dado = TIPO_INT;

    NoAST *params = criarNo(NO_PARAMETROS, 20);

    NoAST *p1 = criarNo(NO_PARAMETRO, 20);
    p1->tipo_dado = TIPO_INT;
    p1->valor.valor_str = strdup("a");
    adicionarFilho(params, p1);

    NoAST *p2 = criarNo(NO_PARAMETRO, 20);
    p2->tipo_dado = TIPO_INT;
    p2->valor.valor_str = strdup("b");
    adicionarFilho(params, p2);

    NoAST *corpo = criarNo(NO_BLOCO, 20);

    adicionarFilho(no_metodo, params);
    adicionarFilho(no_metodo, corpo);

    FILE *f = tmpfile();
    emitirMetodo(no_metodo, f, 0);

    char *saida = lerArquivoTemporario(f);
    fclose(f);

    const char *esperado =
        "int somar(int a, int b) {\n"
        "}\n\n";

    conferir("metodo com multiplos parametros", saida, esperado);

    free(saida);
    liberarAST(no_metodo);
}

static void teste_metodo_com_corpo(void)
{
    NoAST *no_metodo = criarNo(NO_METODO, 30);
    no_metodo->valor.valor_str = strdup("executar");
    no_metodo->tipo_dado = TIPO_VOID;

    NoAST *params = criarNo(NO_PARAMETROS, 30);
    NoAST *corpo = criarNo(NO_BLOCO, 30);

    NoAST *cmd1 = criarNo(NO_BREAK, 31);
    NoAST *cmd2 = criarNo(NO_CONTINUE, 32);
    adicionarFilho(corpo, cmd1);
    adicionarFilho(corpo, cmd2);

    adicionarFilho(no_metodo, params);
    adicionarFilho(no_metodo, corpo);

    FILE *f = tmpfile();
    emitirMetodo(no_metodo, f, 0);

    char *saida = lerArquivoTemporario(f);
    fclose(f);

    const char *esperado =
        "void executar() {\n"
        "\tbreak;\n"
        "\tcontinue;\n"
        "}\n\n";

    conferir("metodo percorre corpo e instrucoes", saida, esperado);

    free(saida);
    liberarAST(no_metodo);
}

static void teste_chamada_metodo(void)
{
    NoAST *chamada = criarNo(NO_CHAMADA_METODO, 40);
    chamada->valor.valor_str = strdup("soma");

    NoAST *arg1 = criarNo(NO_LITERAL_INT, 40);
    arg1->valor.valor_int = 10;
    adicionarFilho(chamada, arg1);

    NoAST *arg2 = criarNo(NO_LITERAL_INT, 40);
    arg2->valor.valor_int = 20;
    adicionarFilho(chamada, arg2);

    FILE *f = tmpfile();
    emitirChamadaMetodo(chamada, f);

    char *saida = lerArquivoTemporario(f);
    fclose(f);

    conferir("emitirChamadaMetodo soma(10, 20)", saida, "soma(10, 20)");

    free(saida);
    liberarAST(chamada);
}

int main(void)
{
    printf("== Testes Unitarios de emitirMetodo e emitirChamadaMetodo ==\n");
    teste_main_vazio();
    teste_metodo_sem_parametros();
    teste_metodo_com_parametros();
    teste_metodo_com_corpo();
    teste_chamada_metodo();

    printf("\nResumo: %d passou, %d falhou (Total: %d)\n", passou, falhas, total);
    return falhas > 0 ? 1 : 0;
}
