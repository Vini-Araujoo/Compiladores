#ifndef SEMANTICA_H
#define SEMANTICA_H

#include "ast.h"
#include "tabela.h"

/* Assinatura de método registrada durante a análise da classe. */
typedef struct AssinaturaMetodo {
    char *nome;
    TipoDado tipo_retorno;
    TipoDado *tipos_parametros;
    int quantidade_parametros;
    struct AssinaturaMetodo *proximo;
} AssinaturaMetodo;

void inicializarTabelaMetodos(void);
void liberarTabelaMetodos(void);
int registrarAssinaturaMetodo(const char *nome, TipoDado retorno,
                              const TipoDado *parametros, int quantidade);
const AssinaturaMetodo *obterAssinaturaMetodo(const char *nome);

/* Inferência conservadora: TIPO_VOID indica expressão desconhecida/inválida. */
TipoDado inferirTipoExpressao(const NoAST *expressao);
int validarAcessoArray(const char *nome, const NoAST *indice, int linha);
int validarChamadaMetodo(const char *nome, const NoAST *argumentos, int linha);

/* Passagem semântica para verificação e resolução de escopos (Issue 8.3) */
int verificarEscopos(NoAST *raiz);

#endif
