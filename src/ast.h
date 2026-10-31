#ifndef AST_H
#define AST_H

#include "tipos.h"
#include "tabela.h"

/*enquanto que na struct guardamos todas de uma vez(tipo produto)
no union guardamos apenas um deles (tipo soma)

portanto so se escolhe um por vez (é alocado o tipo de maior valor)
*/
typedef union
{
    long valor_int;
    double valor_double;
    char *valor_str;
    char operador[4];
} ValorLiteral;

typedef struct NoAST
{
    TipoNoAST tipo_no;  /* Categoria sintática do nó (ex: NO_IF, NO_METODO) */
    TipoDado tipo_dado; /* Tipo associado (ex: TIPO_INT, TIPO_VOID) */
    ValorLiteral valor; /* Valor do literal, nome do identificador ou operador */
    int linha;          /* Linha correspondente no código fonte Java */
    int posfixo;        /* NO_OP_UNARIA: 1 = operador pos-fixado (x++), 0 = pre-fixado (++x, -x, !x) */
    Modificadores mods; /* Modificadores (final, public, static, etc.) */
    struct Simbolo *simbolo; /* Ponteiro para a entrada do símbolo na tabela de símbolos */

    struct NoAST **filhos; /* Vetor dinâmico de ponteiros para nós filhos */
    int num_filhos;        /* Quantidade atual de filhos */
    int capacidade;        /* Capacidade alocada no vetor de filhos */
} NoAST;

NoAST *criarNo(TipoNoAST tipo, int linha);

void adicionarFilho(NoAST *pai, NoAST *filho);

void imprimirAST(const NoAST *raiz);

void liberarAST(NoAST *raiz);

#endif

/*
ex: int x = 10;


ponteiro: no_decl
 ┌──────────────────────────────────────────────┐
 │ tipo_no:    NO_VAR_DECL                      │
 │ tipo_dado:  TIPO_INT                         │
 │ num_filhos: 2                                │
 │ filhos[0] ────────┐   filhos[1] ──────────┐  │
 └───────────────────┼───────────────────────┼──┘
                     │                       │
                     ▼                       ▼
 ┌─────────────────────────────┐ ┌─────────────────────────────┐
 │ tipo_no:    NO_IDENTIFICADOR│ │ tipo_no:    NO_LITERAL_INT  │
 │ tipo_dado:  TIPO_INT        │ │ tipo_dado:  TIPO_INT        │
 │ valor.valor_str: "x"        │ │ valor.valor_int: 10         │
 │ num_filhos: 0 (folha)       │ │ num_filhos: 0 (folha)       │
 └─────────────────────────────┘ └─────────────────────────────┘


*/
