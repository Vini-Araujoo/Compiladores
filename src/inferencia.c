#include <stdio.h>
#include <string.h>
#include "inferencia.h"
#include "ast.h"
#include "tipos.h"

/*
 * Stub da Issue 8.1: ate a tabela de simbolos (buscarSimbolo) estar integrada,
 * o tipo de um identificador vem deste mock.
 */
#ifndef TABELA_INTEGRADA
TipoDado buscarTipoSimboloMock(char *nome) {
    if (nome && strcmp(nome, "flag") == 0) return TIPO_BOOLEAN;
    if (nome && strcmp(nome, "taxa") == 0) return TIPO_DOUBLE;
    return TIPO_INT; // Variáveis padrão são int no stub
}
#endif

static TipoDado tipoDoIdentificador(char *nome) {
    if (!nome) {
        return TIPO_INDEFINIDO;
    }
    return buscarTipoSimboloMock(nome);
}

static int ehNumerico(TipoDado tipo) {
    return tipo == TIPO_BYTE || tipo == TIPO_SHORT || tipo == TIPO_CHAR ||
           tipo == TIPO_INT || tipo == TIPO_LONG || tipo == TIPO_FLOAT ||
           tipo == TIPO_DOUBLE;
}

/*
 * Promocao numerica binaria do Java: se algum operando for double, o resultado
 * e double; senao float, senao long; senao, no minimo int.
 */
static TipoDado promocaoNumerica(TipoDado esquerda, TipoDado direita) {
    if (!ehNumerico(esquerda) || !ehNumerico(direita)) {
        return TIPO_INDEFINIDO;
    }
    if (esquerda == TIPO_DOUBLE || direita == TIPO_DOUBLE) {
        return TIPO_DOUBLE;
    }
    if (esquerda == TIPO_FLOAT || direita == TIPO_FLOAT) {
        return TIPO_FLOAT;
    }
    if (esquerda == TIPO_LONG || direita == TIPO_LONG) {
        return TIPO_LONG;
    }
    return TIPO_INT;
}

static TipoDado inferirOperacaoBinaria(NoAST *no) {
    TipoDado esquerda = inferirTipoExpressao(no->filhos[0]);
    TipoDado direita = inferirTipoExpressao(no->filhos[1]);
    const char *op = no->valor.operador;

    if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
        strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
        strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0 ||
        strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
        return TIPO_BOOLEAN;
    }
    return promocaoNumerica(esquerda, direita);
}

static TipoDado inferirOperacaoUnaria(NoAST *no) {
    TipoDado operando = inferirTipoExpressao(no->filhos[0]);
    const char *op = no->valor.operador;

    if (strcmp(op, "!") == 0) {
        return TIPO_BOOLEAN;
    }
    if (strcmp(op, "-") == 0) {
        return promocaoNumerica(operando, TIPO_INT);
    }
    return ehNumerico(operando) ? operando : TIPO_INDEFINIDO;
}

TipoDado inferirTipoExpressao(NoAST *expr) {
    TipoDado tipo;

    if (!expr) {
        return TIPO_INDEFINIDO;
    }

    switch (expr->tipo_no) {
    case NO_LITERAL_INT:
        tipo = TIPO_INT;
        break;
    case NO_LITERAL_DOUBLE:
        tipo = TIPO_DOUBLE;
        break;
    case NO_LITERAL_BOOL:
        tipo = TIPO_BOOLEAN;
        break;
    case NO_IDENTIFICADOR:
        tipo = tipoDoIdentificador(expr->valor.valor_str);
        break;
    case NO_OP_BINARIA:
        if (expr->num_filhos < 2) {
            return TIPO_INDEFINIDO;
        }
        tipo = inferirOperacaoBinaria(expr);
        break;
    case NO_OP_UNARIA:
        if (expr->num_filhos < 1) {
            return TIPO_INDEFINIDO;
        }
        tipo = inferirOperacaoUnaria(expr);
        break;
    default:
        return TIPO_INDEFINIDO;
    }

    expr->tipo_dado = tipo;
    return tipo;
}

static int ehNoExpressao(const NoAST *no) {
    switch (no->tipo_no) {
    case NO_OP_BINARIA:
    case NO_OP_UNARIA:
    case NO_IDENTIFICADOR:
    case NO_LITERAL_INT:
    case NO_LITERAL_FLOAT:
    case NO_LITERAL_DOUBLE:
    case NO_LITERAL_CHAR:
    case NO_LITERAL_STRING:
    case NO_LITERAL_BOOL:
    case NO_CHAMADA_METODO:
    case NO_NOVA_INSTANCIA:
    case NO_INSTANCIACAO_ARRAY:
    case NO_ACESSO_ARRAY:
    case NO_SCANNER:
    case NO_EXPRESSAO:
        return 1;
    default:
        return 0;
    }
}

int analisarTiposAST(NoAST *raiz) {
    int erros = 0;

    if (!raiz) {
        return 0;
    }

    if (ehNoExpressao(raiz)) {
        if (inferirTipoExpressao(raiz) == TIPO_INDEFINIDO) {
            fprintf(stderr,
                    "Erro semantico na linha %d: tipo da expressao nao pode ser determinado\n",
                    raiz->linha);
            erros++;
        }
        return erros;
    }

    for (int i = 0; i < raiz->num_filhos; i++) {
        erros += analisarTiposAST(raiz->filhos[i]);
    }
    return erros;
}