/*
 * Programa de apoio de tests/codegen/run-tests.sh (Issues 10.7 e 10.8).
 *
 * Le um programa Java em stdin, monta a AST com o parser real e escreve em stdout um programa C
 * que contem a traducao de cada declaracao com inicializador e de cada comando de incremento /
 * decremento, na ordem do fonte, usando emitirExpressao. No fim, imprime o valor de todas as
 * variaveis declaradas, para que o resultado possa ser comparado com a execucao em Java.
 *
 * Isto e andaime de teste: a emissao de declaracoes e de impressao pertence a outras issues.
 */
#include "codegen.h"

extern int yyparse(void);
extern NoAST *arvore;

/*
 * emitirAcessoArray e emitirChamadaMetodo pertencem a outros modulos do gerador. Enquanto nao
 * existirem, estas definicoes fracas permitem ligar o teste; quando forem implementadas, a
 * definicao forte delas prevalece no link, sem alterar este arquivo.
 */
__attribute__((weak)) void emitirAcessoArray(NoAST *no, FILE *saida)
{
    (void)no;
    (void)saida;
    fprintf(stderr, "driver_expr: emitirAcessoArray ainda nao implementada\n");
}

__attribute__((weak)) void emitirChamadaMetodo(NoAST *no, FILE *saida)
{
    (void)no;
    (void)saida;
    fprintf(stderr, "driver_expr: emitirChamadaMetodo ainda nao implementada\n");
}

#define MAX_VARIAVEIS 512

typedef struct
{
    const char *nome;
    TipoDado tipo;
} Variavel;

static Variavel variaveis[MAX_VARIAVEIS];
static int num_variaveis = 0;

static const char *tipoC(TipoDado tipo)
{
    switch (tipo)
    {
    case TIPO_DOUBLE:
        return "double";
    case TIPO_BOOLEAN:
        return "bool";
    case TIPO_STRING:
        return "const char *";
    default:
        return "int";
    }
}

static void percorrer(NoAST *no, FILE *saida)
{
    if (no == NULL)
        return;

    switch (no->tipo_no)
    {
    case NO_DECL_VARIAVEL:
        if (no->valor.valor_str != NULL && no->num_filhos == 1)
        {
            if (num_variaveis >= MAX_VARIAVEIS)
            {
                fprintf(stderr, "driver_expr: variaveis demais\n");
                exit(2);
            }
            fprintf(saida, "    %s %s = ", tipoC(no->tipo_dado), no->valor.valor_str);
            emitirExpressao(no->filhos[0], saida);
            fprintf(saida, ";\n");
            variaveis[num_variaveis].nome = no->valor.valor_str;
            variaveis[num_variaveis].tipo = no->tipo_dado;
            num_variaveis++;
            return;
        }
        break;
    case NO_INCREMENTO:
    case NO_DECREMENTO:
    case NO_INCREMENTO_ANTES:
    case NO_DECREMENTO_ANTES:
        fprintf(saida, "    ");
        emitirExpressao(no, saida);
        fprintf(saida, ";\n");
        return;
    default:
        break;
    }

    for (int i = 0; i < no->num_filhos; i++)
        percorrer(no->filhos[i], saida);
}

static void imprimirVariaveis(FILE *saida)
{
    for (int i = 0; i < num_variaveis; i++)
    {
        const char *nome = variaveis[i].nome;
        switch (variaveis[i].tipo)
        {
        case TIPO_DOUBLE:
            fprintf(saida, "    printf(\"%%g\\n\", %s);\n", nome);
            break;
        case TIPO_BOOLEAN:
            fprintf(saida, "    puts(%s ? \"true\" : \"false\");\n", nome);
            break;
        case TIPO_STRING:
            fprintf(saida, "    puts(%s);\n", nome);
            break;
        default:
            fprintf(saida, "    printf(\"%%d\\n\", %s);\n", nome);
            break;
        }
    }
}

int main(void)
{
    if (yyparse() != 0)
        return 1;

    emitirCabecalho(stdout);
    printf("int main(void) {\n");
    percorrer(arvore, stdout);
    imprimirVariaveis(stdout);
    printf("    return 0;\n}\n");

    liberarAST(arvore);
    return 0;
}