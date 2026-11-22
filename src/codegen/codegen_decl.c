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

static int operadorAtribuicaoValido(const char *op)
{
    if (op == NULL)
    {
        return 0;
    }
    return strcmp(op, "=") == 0 ||
           strcmp(op, "+=") == 0 ||
           strcmp(op, "-=") == 0 ||
           strcmp(op, "*=") == 0 ||
           strcmp(op, "/=") == 0 ||
           strcmp(op, "%=") == 0;
}

static int emitirLadoEsquerdoAtribuicao(NoAST *alvo, FILE *saida, int linha)
{
    if (alvo == NULL)
    {
        fprintf(stderr, "Erro de geracao na linha %d: alvo de atribuicao nulo.\n", linha);
        return 0;
    }

    if (alvo->tipo_no == NO_IDENTIFICADOR)
    {
        if (alvo->valor.valor_str == NULL)
        {
            fprintf(stderr, "Erro de geracao na linha %d: identificador sem nome na atribuicao.\n", linha);
            return 0;
        }
        fprintf(saida, "%s", alvo->valor.valor_str);
        return 1;
    }

    if (alvo->tipo_no == NO_ACESSO_ARRAY)
    {
        emitirAcessoArray(alvo, saida);
        return 1;
    }

    emitirExpressao(alvo, saida);
    return 1;
}

void emitirAtribuicao(NoAST *no, FILE *saida, int nivel)
{
    if (no == NULL || saida == NULL)
    {
        return;
    }

    /* Comandos de incremento e decremento oriundos da regra atribuicao */
    if (no->tipo_no == NO_INCREMENTO || no->tipo_no == NO_DECREMENTO ||
        no->tipo_no == NO_INCREMENTO_ANTES || no->tipo_no == NO_DECREMENTO_ANTES)
    {
        if (no->num_filhos < 1 || no->filhos[0] == NULL)
        {
            fprintf(stderr, "Erro de geracao na linha %d: operacao de incremento/decremento sem operando.\n",
                    no->linha);
            return;
        }
        const char *op_inc = (no->tipo_no == NO_INCREMENTO || no->tipo_no == NO_INCREMENTO_ANTES)
                                 ? "++"
                                 : "--";
        emitirIndentacao(saida, nivel);
        if (no->tipo_no == NO_INCREMENTO_ANTES || no->tipo_no == NO_DECREMENTO_ANTES)
        {
            fprintf(saida, "%s", op_inc);
            emitirLadoEsquerdoAtribuicao(no->filhos[0], saida, no->linha);
        }
        else
        {
            emitirLadoEsquerdoAtribuicao(no->filhos[0], saida, no->linha);
            fprintf(saida, "%s", op_inc);
        }
        fprintf(saida, ";\n");
        return;
    }

    if (no->tipo_no != NO_ATRIBUICAO)
    {
        fprintf(stderr, "Erro de geracao na linha %d: no inesperado em emitirAtribuicao.\n",
                no->linha);
        return;
    }

    if (no->num_filhos < 2 || no->filhos[0] == NULL || no->filhos[1] == NULL)
    {
        fprintf(stderr, "Erro de geracao na linha %d: atribuicao incompleta na AST.\n",
                no->linha);
        return;
    }

    NoAST *alvo = no->filhos[0];
    NoAST *expr = no->filhos[1];

    /* Caso especial: leitura de Scanner atribuida a variavel existente (ex.: x = sc.nextInt();) */
    NoAST *leitura = obterLeituraScanner(expr);
    if (leitura != NULL)
    {
        emitirScannerLeitura(leitura, saida, nivel);
        return;
    }

    const char *op = (no->valor.operador[0] != '\0') ? no->valor.operador : "=";
    if (!operadorAtribuicaoValido(op))
    {
        fprintf(stderr, "Erro de geracao na linha %d: operador de atribuicao invalido '%s'.\n",
                no->linha, op);
        return;
    }

    if (alvo->tipo_no == NO_IDENTIFICADOR && alvo->valor.valor_str == NULL)
    {
        fprintf(stderr, "Erro de geracao na linha %d: identificador sem nome na atribuicao.\n",
                no->linha);
        return;
    }

    emitirIndentacao(saida, nivel);
    emitirLadoEsquerdoAtribuicao(alvo, saida, no->linha);
    fprintf(saida, " %s ", op);
    emitirExpressao(expr, saida);
    fprintf(saida, ";\n");
}
