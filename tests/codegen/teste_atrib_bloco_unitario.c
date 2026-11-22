#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"

/* Prototipo de emitirBloco (implementada em src/codegen/codegen_decl.c) */
void emitirBloco(NoAST *no, FILE *saida, int nivel);

static int total = 0;
static int falhas = 0;

static int chamadas_acesso_array = 0;
static int chamadas_scanner_leitura = 0;

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
        case NO_LITERAL_STRING:
            fprintf(saida, "\"%s\"", no->valor.valor_str ? no->valor.valor_str : "");
            break;
        case NO_OP_BINARIA:
            fprintf(saida, "(");
            emitirExpressao(no->filhos[0], saida);
            fprintf(saida, " %s ", no->valor.operador);
            emitirExpressao(no->filhos[1], saida);
            fprintf(saida, ")");
            break;
        case NO_ACESSO_ARRAY:
            emitirAcessoArray(no, saida);
            break;
        default:
            fprintf(saida, "/*expr*/");
            break;
    }
}

/* Stub fraco de emitirAcessoArray (modulo de Natan Franca) */
__attribute__((weak)) void emitirAcessoArray(NoAST *no, FILE *saida)
{
    chamadas_acesso_array++;
    if (no == NULL || no->num_filhos < 2)
    {
        return;
    }
    emitirExpressao(no->filhos[0], saida);
    fprintf(saida, "[");
    emitirExpressao(no->filhos[1], saida);
    fprintf(saida, "]");
}

/* Stub fraco de emitirScannerLeitura (modulo de Natan Franca) */
__attribute__((weak)) void emitirScannerLeitura(NoAST *no, FILE *saida, int nivel)
{
    chamadas_scanner_leitura++;
    (void)no;
    emitirIndentacao(saida, nivel);
    fprintf(saida, "scanf(\"%%d\", &x);\n");
}

/* Stub fraco de emitirInstrucao (orquestrador em codegen_main.c) */
__attribute__((weak)) void emitirInstrucao(NoAST *no, FILE *saida, int nivel)
{
    if (no == NULL)
    {
        return;
    }
    switch (no->tipo_no)
    {
        case NO_DECL_VARIAVEL:
            emitirDeclaracaoVariavel(no, saida, nivel);
            break;
        case NO_ATRIBUICAO:
        case NO_INCREMENTO:
        case NO_DECREMENTO:
        case NO_INCREMENTO_ANTES:
        case NO_DECREMENTO_ANTES:
            emitirAtribuicao(no, saida, nivel);
            break;
        case NO_BLOCO:
            emitirBloco(no, saida, nivel);
            break;
        default:
            emitirLinha(saida, nivel, "/* instrucao */");
            break;
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

static NoAST *criarId(const char *nome)
{
    NoAST *no = criarNo(NO_IDENTIFICADOR, 1);
    no->valor.valor_str = strdup(nome);
    return no;
}

static NoAST *criarLitInt(long v)
{
    NoAST *no = criarNo(NO_LITERAL_INT, 1);
    no->tipo_dado = TIPO_INT;
    no->valor.valor_int = v;
    return no;
}

static NoAST *criarOpBin(const char *op, NoAST *esq, NoAST *dir)
{
    NoAST *no = criarNo(NO_OP_BINARIA, 1);
    strncpy(no->valor.operador, op, sizeof(no->valor.operador) - 1);
    adicionarFilho(no, esq);
    adicionarFilho(no, dir);
    return no;
}

static NoAST *criarAcessoArray(const char *nome, NoAST *indice)
{
    NoAST *no = criarNo(NO_ACESSO_ARRAY, 1);
    adicionarFilho(no, criarId(nome));
    adicionarFilho(no, indice);
    return no;
}

static NoAST *criarAtrib(const char *op, NoAST *alvo, NoAST *expr)
{
    NoAST *no = criarNo(NO_ATRIBUICAO, 1);
    if (op != NULL)
    {
        strncpy(no->valor.operador, op, sizeof(no->valor.operador) - 1);
    }
    adicionarFilho(no, alvo);
    adicionarFilho(no, expr);
    return no;
}

static NoAST *criarDeclInt(const char *nome, long valor)
{
    NoAST *no = criarNo(NO_DECL_VARIAVEL, 1);
    no->tipo_dado = TIPO_INT;
    no->valor.valor_str = strdup(nome);
    adicionarFilho(no, criarLitInt(valor));
    return no;
}

static void testarAtrib(const char *nome_teste, NoAST *no, int nivel, const char *esperado)
{
    FILE *f = tmpfile();
    assert(f != NULL);
    emitirAtribuicao(no, f, nivel);
    char *obtido = lerArquivoTemporario(f);
    fclose(f);

    conferirTexto(nome_teste, obtido, esperado);
    free(obtido);
    liberarAST(no);
}

static void testarBloco(const char *nome_teste, NoAST *no, int nivel, const char *esperado)
{
    FILE *f = tmpfile();
    assert(f != NULL);
    emitirBloco(no, f, nivel);
    char *obtido = lerArquivoTemporario(f);
    fclose(f);

    conferirTexto(nome_teste, obtido, esperado);
    free(obtido);
    liberarAST(no);
}

int main(void)
{
    printf("== Teste 1: Atribuicao Simples e Precedencia ==\n");
    testarAtrib("x = 10; (nivel 1)",
                criarAtrib("=", criarId("x"), criarLitInt(10)), 1,
                "\tx = 10;\n");
    testarAtrib("operador vazio assume '='",
                criarAtrib("", criarId("y"), criarLitInt(5)), 0,
                "y = 5;\n");
    testarAtrib("r = (a + b) * 2; (precedencia)",
                criarAtrib("=", criarId("r"),
                           criarOpBin("*", criarOpBin("+", criarId("a"), criarId("b")), criarLitInt(2))),
                1, "\tr = ((a + b) * 2);\n");

    printf("\n== Teste 2: Todas as Atribuicoes Compostas (=, +=, -=, *=, /=, %%=) ==\n");
    testarAtrib("x += 5;", criarAtrib("+=", criarId("x"), criarLitInt(5)), 1, "\tx += 5;\n");
    testarAtrib("x -= 3;", criarAtrib("-=", criarId("x"), criarLitInt(3)), 1, "\tx -= 3;\n");
    testarAtrib("x *= (a + 1);",
                criarAtrib("*=", criarId("x"), criarOpBin("+", criarId("a"), criarLitInt(1))),
                1, "\tx *= (a + 1);\n");
    testarAtrib("x /= 2;", criarAtrib("/=", criarId("x"), criarLitInt(2)), 1, "\tx /= 2;\n");
    testarAtrib("x %= 4;", criarAtrib("%=", criarId("x"), criarLitInt(4)), 2, "\t\tx %= 4;\n");

    printf("\n== Teste 3: Atribuicao com Acesso a Array (NO_ACESSO_ARRAY) ==\n");
    chamadas_acesso_array = 0;
    testarAtrib("vetor[0] = 42;",
                criarAtrib("=", criarAcessoArray("vetor", criarLitInt(0)), criarLitInt(42)),
                1, "\tvetor[0] = 42;\n");
    testarAtrib("vetor[(i + 1)] += 10;",
                criarAtrib("+=", criarAcessoArray("vetor", criarOpBin("+", criarId("i"), criarLitInt(1))),
                           criarLitInt(10)),
                1, "\tvetor[(i + 1)] += 10;\n");
    testarAtrib("notas[i] *= 2;",
                criarAtrib("*=", criarAcessoArray("notas", criarId("i")), criarLitInt(2)),
                1, "\tnotas[i] *= 2;\n");
    total++;
    if (chamadas_acesso_array == 3)
    {
        printf("  [PASS] delegacao para emitirAcessoArray verificada (3 chamadas)\n");
    }
    else
    {
        printf("  [FAIL] esperado 3 chamadas a emitirAcessoArray, obtido %d\n", chamadas_acesso_array);
        falhas++;
    }

    printf("\n== Teste 4: Incrementos, Decrementos e Leitura de Scanner ==\n");
    {
        NoAST *inc = criarNo(NO_INCREMENTO, 1);
        adicionarFilho(inc, criarId("i"));
        testarAtrib("i++;", inc, 1, "\ti++;\n");

        NoAST *pre = criarNo(NO_INCREMENTO_ANTES, 1);
        adicionarFilho(pre, criarId("i"));
        testarAtrib("++i;", pre, 1, "\t++i;\n");

        NoAST *dec = criarNo(NO_DECREMENTO, 1);
        adicionarFilho(dec, criarId("j"));
        testarAtrib("j--;", dec, 1, "\tj--;\n");

        NoAST *pre_dec = criarNo(NO_DECREMENTO_ANTES, 1);
        adicionarFilho(pre_dec, criarId("j"));
        testarAtrib("--j;", pre_dec, 1, "\t--j;\n");
    }
    {
        chamadas_scanner_leitura = 0;
        NoAST *leitura = criarNo(NO_LEITURA_SCANNER, 1);
        leitura->valor.valor_str = strdup("nextInt");
        adicionarFilho(leitura, criarId("sc"));
        testarAtrib("x = sc.nextInt(); delega para emitirScannerLeitura",
                    criarAtrib("=", criarId("x"), leitura), 1,
                    "\tscanf(\"%d\", &x);\n");
        total++;
        if (chamadas_scanner_leitura == 1)
        {
            printf("  [PASS] delegacao para emitirScannerLeitura em atribuicao verificada\n");
        }
        else
        {
            printf("  [FAIL] esperado 1 chamada a emitirScannerLeitura\n");
            falhas++;
        }
    }

    printf("\n== Teste 5: Blocos de Escopo (emitirBloco) e Aninhamento ==\n");
    testarBloco("bloco vazio nivel 0", criarNo(NO_BLOCO, 1), 0, "{\n}\n");
    testarBloco("bloco vazio nivel 1", criarNo(NO_BLOCO, 1), 1, "\t{\n\t}\n");
    {
        NoAST *interno2 = criarNo(NO_BLOCO, 4);
        adicionarFilho(interno2, criarDeclInt("x", 30));
        adicionarFilho(interno2, criarAtrib("*=", criarId("x"), criarLitInt(2)));

        NoAST *interno1 = criarNo(NO_BLOCO, 2);
        adicionarFilho(interno1, criarDeclInt("x", 20));
        adicionarFilho(interno1, interno2);
        adicionarFilho(interno1, criarAtrib("-=", criarId("x"), criarLitInt(5)));

        NoAST *externo = criarNo(NO_BLOCO, 1);
        adicionarFilho(externo, criarDeclInt("x", 10));
        adicionarFilho(externo, interno1);
        adicionarFilho(externo, criarAtrib("+=", criarId("x"), criarLitInt(1)));

        const char *esperado =
            "{\n"
            "\tint x = 10;\n"
            "\t{\n"
            "\t\tint x = 20;\n"
            "\t\t{\n"
            "\t\t\tint x = 30;\n"
            "\t\t\tx *= 2;\n"
            "\t\t}\n"
            "\t\tx -= 5;\n"
            "\t}\n"
            "\tx += 1;\n"
            "}\n";

        testarBloco("blocos aninhados em 3 niveis com shadowing e indentacao", externo, 0, esperado);
    }

    printf("\n== Teste 6: Casos Defensivos ==\n");
    {
        FILE *f = tmpfile();
        assert(f != NULL);
        emitirAtribuicao(NULL, f, 1);
        emitirBloco(NULL, f, 1);
        char *obtido = lerArquivoTemporario(f);
        fclose(f);
        conferirTexto("nos nulos nao emitem nada", obtido, "");
        free(obtido);
    }

    printf("\nResumo teste_atrib_bloco_unitario: %d passou, %d falhou.\n", total - falhas, falhas);
    return falhas == 0 ? 0 : 1;
}
