#include "codegen.h"
#include <stdarg.h>

/* Stubs fracos auxiliares ainda n impl pelo moretti */
__attribute__((weak)) void emitirPrint(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}
__attribute__((weak)) void emitirPrintln(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}
__attribute__((weak)) void emitirIf(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}
__attribute__((weak)) void emitirWhile(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}
__attribute__((weak)) void emitirDoWhile(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}
__attribute__((weak)) void emitirFor(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}
__attribute__((weak)) void emitirSwitch(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}

void emitirIndentacao(FILE *saida, int nivel)
{
    for (int i = 0; i < nivel; i++)
    {
        fprintf(saida, "\t");
    }
}

// Em vez de void emitirLinha(...
void emitirCabecalho(FILE *saida)
{
    fprintf(saida, "#include <stdio.h>\n");
    fprintf(saida, "#include <stdlib.h>\n");
    fprintf(saida, "#include <stdbool.h>\n");
    fprintf(saida, "#include <string.h>\n\n");
}
void emitirLinha(FILE *saida, int nivel, const char *format, ...)
{
    emitirIndentacao(saida, nivel);

    va_list args;

    va_start(args, format);        // começa a ler tudo que vem depois do format(apartir do ...) salva no struct
    vfprintf(saida, format, args); // le e preenche os tipos
    va_end(args);

    fprintf(saida, "\n");
}

void emitirMetodo(NoAST *no, FILE *saida, int nivel)
{
    if (no != NULL && no->tipo_no == NO_METODO)
    {

        if (strcmp(no->valor.valor_str, "main") == 0)
        {
            emitirLinha(saida, nivel, "int main(int argc, char *argv[]){");

            NoAST *corpo = no->filhos[1];
            if (corpo)
            {
                for (int i = 0; i < corpo->num_filhos; i++)
                {
                    NoAST *comando = corpo->filhos[i];

                    emitirInstrucao(comando, saida, nivel + 1);
                }
            }
            emitirLinha(saida, nivel + 1, "return 0;");
            emitirLinha(saida, nivel, "}\n");
        }
        else
        {
            const char *retornoMetodo = traduzirTipoC(no->tipo_dado);
            const char *nomeMetodo = no->valor.valor_str;

            emitirIndentacao(saida, nivel);
            fprintf(saida, "%s %s(", retornoMetodo, nomeMetodo);

            NoAST *parametros = no->filhos[0]; // o filho 0 sao os param do metodo

            if (parametros != NULL)
            {
                // percorremos os filhos dos parametros identificando tipo e nome e logo printamos
                for (int i = 0; i < parametros->num_filhos; i++)
                {
                    NoAST *p = parametros->filhos[i];
                    const char *tipoParam = traduzirTipoC(p->tipo_dado);
                    const char *nomeParam = p->valor.valor_str;

                    if (i > 0)
                    {
                        fprintf(saida, ", ");
                    }
                    fprintf(saida, "%s %s", tipoParam, nomeParam);
                }
            }
            fprintf(saida, ") {\n"); //) e { do metodo

            // termina de imprimir o resto

            NoAST *corpo = no->filhos[1];
            if (corpo)
            {
                for (int i = 0; i < corpo->num_filhos; i++)
                {
                    NoAST *comando = corpo->filhos[i];

                    emitirInstrucao(comando, saida, nivel + 1);
                }
            }

            emitirLinha(saida, nivel, "}\n");
        }
    }
}

// responsavel por tratar a chamada de um metodo ex: int total = soma(10, x+2);
void emitirChamadaMetodo(NoAST *no, FILE *saida)
{
    if (no != NULL && no->valor.valor_str != NULL)
    {
        fprintf(saida, "%s(", no->valor.valor_str);

        for (int i = 0; i < no->num_filhos; i++)
        {
            // coloca ',' antes de emitir exp
            if (i > 0)
            {
                fprintf(saida, ", ");
            }

            emitirExpressao(no->filhos[i], saida);
        }

        fprintf(saida, ")");
    }
}

//__attribute__((weak)) em func serve pra indicar que aquela func foi definida como stub eai quando criar uma função com o mesmo nome e param ele utilizara a outra

void emitirInstrucao(NoAST *no, FILE *saida, int nivel)
{
    // the big switch is watching you
    // nesse switch apenas
    if (no != NULL)
    {
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

        case NO_IF:
            emitirIf(no, saida, nivel);
            break;

        case NO_WHILE:
            emitirWhile(no, saida, nivel);
            break;

        case NO_DO_WHILE:
            emitirDoWhile(no, saida, nivel);
            break;

        case NO_FOR:
            emitirFor(no, saida, nivel);
            break;

        case NO_SWITCH:
            emitirSwitch(no, saida, nivel);
            break;
        case NO_PRINT:
            emitirPrint(no, saida, nivel);

            break;
        case NO_PRINTLN:
            emitirPrintln(no, saida, nivel);
            break;

        case NO_LEITURA_SCANNER:
            emitirScannerLeitura(no, saida, nivel);
            break;

        case NO_CHAMADA_METODO:
            emitirIndentacao(saida, nivel);
            emitirChamadaMetodo(no, saida);
            fprintf(saida, ";\n");
            break;

            // quando o return n tem filhos (como ex: return x+2) ai apenas imprimimos o return direto (else)
            // caso tenha filhos ai repassamos pro emitirExpressao
        case NO_RETURN:
            if (no->num_filhos > 0 && no->filhos[0] != NULL)
            {
                emitirIndentacao(saida, nivel);
                fprintf(saida, "return ");
                emitirExpressao(no->filhos[0], saida);
                fprintf(saida, ";\n");
            }
            else
            {
                emitirLinha(saida, nivel, "return;");
            }
            break;

        case NO_BREAK:
            emitirLinha(saida, nivel, "break;");
            break;

        case NO_CONTINUE:
            emitirLinha(saida, nivel, "continue;");
            break;

        case NO_EXPRESSAO:
            // caso a exp esteja embrulhada com filhos
            if (no->num_filhos > 0 && no->filhos[0] != NULL)
            {
                // Desembrulha e manda o comando real ser processado pelo switch!
                emitirInstrucao(no->filhos[0], saida, nivel);
            }
            break;
        default:
            break;
        }
    }
}

void gerarCodigoC(NoAST *raiz, FILE *saida)
{
    (void)raiz;
    (void)saida;
}

/*
Se NÃO for "main":
Descobre o tipo de retorno em C (usando o traduzirTipoC(no->tipo_dado) que o Vinícius implementou)
Emite o tipo, o nome da função e abre parênteses (
Percorre os parâmetros em no->filhos[0] emitindo tipo nome, tipo nome
Abre a chave {
Percorre as instruções do corpo (no->filhos[1])
Fecha com }
*/