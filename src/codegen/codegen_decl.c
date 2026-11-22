#include "codegen.h"

const char *traduzirTipoC(TipoDado tipo)
{
    switch (tipo)
    {
        case TIPO_BYTE:
            return "char";
        case TIPO_SHORT:
            return "short";
        case TIPO_INT:
            return "int";
        case TIPO_LONG:
            return "long long";
        case TIPO_FLOAT:
            return "float";
        case TIPO_DOUBLE:
            return "double";
        case TIPO_CHAR:
            return "char";
        case TIPO_BOOLEAN:
            return "bool";
        case TIPO_STRING:
            return "char *";
        case TIPO_VOID:
            return "void";
        case TIPO_SCANNER:
            return NULL;
        case TIPO_ARRAY:
        case TIPO_INDEFINIDO:
        default:
            return NULL;
    }
}

static int noEhFinal(const NoAST *no)
{
    if (no == NULL)
    {
        return 0;
    }
    if (no->mods.eh_final)
    {
        return 1;
    }
    if (no->simbolo != NULL && (no->simbolo->eh_final || no->simbolo->mods.eh_final))
    {
        return 1;
    }
    return 0;
}

static int ehContainerDeclaracoes(const NoAST *no)
{
    if (no == NULL || no->valor.valor_str != NULL || no->num_filhos <= 0)
    {
        return 0;
    }
    for (int i = 0; i < no->num_filhos; i++)
    {
        if (no->filhos[i] != NULL)
        {
            return no->filhos[i]->tipo_no == NO_DECL_VARIAVEL;
        }
    }
    return 0;
}

static NoAST *obterLeituraScanner(NoAST *expr)
{
    if (expr == NULL)
    {
        return NULL;
    }
    if (expr->tipo_no == NO_LEITURA_SCANNER)
    {
        return expr;
    }
    if (expr->tipo_no == NO_EXPRESSAO && expr->num_filhos == 1 &&
        expr->filhos[0] != NULL && expr->filhos[0]->tipo_no == NO_LEITURA_SCANNER)
    {
        return expr->filhos[0];
    }
    return NULL;
}

static void emitirTipoENome(FILE *saida, const char *tipo_c, const char *nome)
{
    size_t tam = strlen(tipo_c);
    if (tam > 0 && tipo_c[tam - 1] == '*')
    {
        fprintf(saida, "%s%s", tipo_c, nome);
    }
    else
    {
        fprintf(saida, "%s %s", tipo_c, nome);
    }
}

static void emitirDeclaracaoInterna(NoAST *no, TipoDado tipo_herdado, int final_herdado,
                                    FILE *saida, int nivel)
{
    if (no == NULL || saida == NULL)
    {
        return;
    }

    if (no->tipo_no != NO_DECL_VARIAVEL)
    {
        fprintf(stderr, "Erro de geracao na linha %d: no inesperado em emitirDeclaracaoVariavel.\n",
                no->linha);
        return;
    }

    /* Suporte a conteineres de declaracoes multiplas (ex.: int a = 1, b, c = 3;) */
    if (ehContainerDeclaracoes(no))
    {
        TipoDado tipo_pai = no->tipo_dado;
        if (tipo_pai == TIPO_VOID || tipo_pai == TIPO_INDEFINIDO)
        {
            tipo_pai = tipo_herdado;
        }
        int final_pai = final_herdado || noEhFinal(no);

        for (int i = 0; i < no->num_filhos; i++)
        {
            if (no->filhos[i] != NULL)
            {
                emitirDeclaracaoInterna(no->filhos[i], tipo_pai, final_pai, saida, nivel);
            }
        }
        return;
    }

    const char *nome = NULL;
    NoAST *inicializador = NULL;
    int eh_final = final_herdado || noEhFinal(no);

    if (no->valor.valor_str != NULL)
    {
        nome = no->valor.valor_str;
        if (no->num_filhos >= 1)
        {
            inicializador = no->filhos[0];
        }
    }
    else if (no->num_filhos >= 1 && no->filhos[0] != NULL &&
             no->filhos[0]->tipo_no == NO_IDENTIFICADOR)
    {
        nome = no->filhos[0]->valor.valor_str;
        if (!eh_final && noEhFinal(no->filhos[0]))
        {
            eh_final = 1;
        }
        if (no->num_filhos >= 2)
        {
            inicializador = no->filhos[1];
        }
    }

    if (nome == NULL)
    {
        fprintf(stderr, "Erro de geracao na linha %d: declaracao de variavel sem identificador.\n",
                no->linha);
        return;
    }

    TipoDado tipo = no->tipo_dado;
    if ((tipo == TIPO_VOID || tipo == TIPO_INDEFINIDO) && no->simbolo != NULL)
    {
        tipo = no->simbolo->tipo;
    }
    if ((tipo == TIPO_VOID || tipo == TIPO_INDEFINIDO) &&
        tipo_herdado != TIPO_VOID && tipo_herdado != TIPO_INDEFINIDO)
    {
        tipo = tipo_herdado;
    }

    /* Caso especial: Scanner nao requer struct em C (<stdio.h> ja fornece stdin) */
    if (tipo == TIPO_SCANNER)
    {
        emitirLinha(saida, nivel, "/* Scanner %s */", nome);
        return;
    }

    const char *tipo_c = traduzirTipoC(tipo);
    if (tipo_c == NULL)
    {
        fprintf(stderr, "Erro de geracao na linha %d: tipo nao suportado na declaracao de '%s'.\n",
                no->linha, nome);
        return;
    }

    /* Caso especial: inicializacao com leitura de Scanner (ex.: int x = sc.nextInt();) */
    NoAST *leitura = obterLeituraScanner(inicializador);
    if (leitura != NULL)
    {
        emitirIndentacao(saida, nivel);
        if (eh_final)
        {
            fprintf(saida, "const ");
        }
        if (tipo == TIPO_STRING)
        {
            fprintf(saida, "char %s[256];\n", nome);
        }
        else
        {
            emitirTipoENome(saida, tipo_c, nome);
            fprintf(saida, ";\n");
        }
        emitirScannerLeitura(leitura, saida, nivel);
        return;
    }

    /* Caso normal: declaracao simples ou com expressao inicializadora */
    emitirIndentacao(saida, nivel);
    if (eh_final)
    {
        fprintf(saida, "const ");
    }
    emitirTipoENome(saida, tipo_c, nome);

    if (inicializador != NULL)
    {
        fprintf(saida, " = ");
        emitirExpressao(inicializador, saida);
    }
    fprintf(saida, ";\n");
}

void emitirDeclaracaoVariavel(NoAST *no, FILE *saida, int nivel)
{
    emitirDeclaracaoInterna(no, TIPO_INDEFINIDO, 0, saida, nivel);
}
