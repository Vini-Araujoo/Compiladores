#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "ast.h"

/* Stubs fracos para dependencias externas do codegen_decl */
void emitirExpressao(NoAST *no, FILE *saida) { (void)no; (void)saida; }
void emitirScannerLeitura(NoAST *no, FILE *saida, int nivel) { (void)no; (void)saida; (void)nivel; }
void emitirAcessoArray(NoAST *no, FILE *saida) { (void)no; (void)saida; }

static int total = 0;
static int passou = 0;
static int falhas = 0;

/* Stub de emitirInstrucao para testar se os comandos do corpo sao chamados */
static int instrucoes_chamadas = 0;
void emitirInstrucao(NoAST *no, FILE *saida, int nivel)
{
    if (no == NULL) return;
    instrucoes_chamadas++;
    emitirLinha(saida, nivel, "/* comando executado */");
}

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
        "int main(int argc, char *argv[]){\n"
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
    instrucoes_chamadas = 0;

    NoAST *no_metodo = criarNo(NO_METODO, 30);
    no_metodo->valor.valor_str = strdup("executar");
    no_metodo->tipo_dado = TIPO_VOID;

    NoAST *params = criarNo(NO_PARAMETROS, 30);
    NoAST *corpo = criarNo(NO_BLOCO, 30);

    NoAST *cmd1 = criarNo(NO_EXPRESSAO, 31);
    NoAST *cmd2 = criarNo(NO_EXPRESSAO, 32);
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
        "\t/* comando executado */\n"
        "\t/* comando executado */\n"
        "}\n\n";

    conferir("metodo percorre corpo e instrucoes", saida, esperado);
    assert(instrucoes_chamadas == 2);

    free(saida);
    liberarAST(no_metodo);
}

int main(void)
{
    printf("== Testes Unitarios de emitirMetodo (codegen_main.c) ==\n");
    teste_main_vazio();
    teste_metodo_sem_parametros();
    teste_metodo_com_parametros();
    teste_metodo_com_corpo();

    printf("\nResumo: %d passou, %d falhou (Total: %d)\n", passou, falhas, total);
    return falhas > 0 ? 1 : 0;
}
