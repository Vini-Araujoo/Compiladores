#include <stdio.h>
#include <string.h>
#include "inferencia.h"
#include "ast.h"
#include "tipos.h"


#ifndef TABELA_INTEGRADA
TipoDado buscarTipoSimboloMock(char *nome) {
    if (nome && strcmp(nome, "flag") == 0) return TIPO_BOOLEAN;
    if (nome && strcmp(nome, "taxa") == 0) return TIPO_DOUBLE;
    return TIPO_INT; 
}
#endif

static TipoDado tipoDoIdentificador(char *nome) {
    if (!nome) return TIPO_INDEFINIDO;
    return buscarTipoSimboloMock(nome);
}

static int ehNumerico(TipoDado tipo) {
    return tipo == TIPO_BYTE || tipo == TIPO_SHORT || tipo == TIPO_CHAR ||
           tipo == TIPO_INT || tipo == TIPO_LONG || tipo == TIPO_FLOAT ||
           tipo == TIPO_DOUBLE;
}

static TipoDado promocaoNumerica(TipoDado esquerda, TipoDado direita) {
    if (!ehNumerico(esquerda) || !ehNumerico(direita)) return TIPO_INDEFINIDO;
    if (esquerda == TIPO_DOUBLE || direita == TIPO_DOUBLE) return TIPO_DOUBLE;
    if (esquerda == TIPO_FLOAT || direita == TIPO_FLOAT) return TIPO_FLOAT;
    if (esquerda == TIPO_LONG || direita == TIPO_LONG) return TIPO_LONG;
    return TIPO_INT;
}

static TipoDado inferirOperacaoBinaria(NoAST *no) {
    TipoDado esquerda = inferirTipoExpressao(no->filhos[0]);
    TipoDado direita = inferirTipoExpressao(no->filhos[1]);
    const char *op = no->valor.operador;

    if (strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
        if (esquerda != TIPO_BOOLEAN || direita != TIPO_BOOLEAN) {
            fprintf(stderr, "Erro semantico na linha %d: Operador logico '%s' aplicado a tipos incompativeis.\n", no->linha, op);
            return TIPO_INDEFINIDO;
        }
        return TIPO_BOOLEAN;
    }
    
    if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 || 
        strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) {
        if (!ehNumerico(esquerda) || !ehNumerico(direita)) {
            fprintf(stderr, "Erro semantico na linha %d: Operador relacional '%s' aplicado a tipos incompativeis (esperava numericos).\n", no->linha, op);
            return TIPO_INDEFINIDO;
        }
        return TIPO_BOOLEAN;
    }

    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || 
        strcmp(op, "*") == 0 || strcmp(op, "/") == 0 || strcmp(op, "%") == 0) {
        if (!ehNumerico(esquerda) || !ehNumerico(direita)) {
             fprintf(stderr, "Erro semantico na linha %d: Operador aritmetico '%s' aplicado a tipos incompativeis.\n", no->linha, op);
             return TIPO_INDEFINIDO;
        }
        return promocaoNumerica(esquerda, direita);
    }

    return promocaoNumerica(esquerda, direita);
}

static TipoDado inferirOperacaoUnaria(NoAST *no) {
    TipoDado operando = inferirTipoExpressao(no->filhos[0]);
    const char *op = no->valor.operador;

    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 || 
        strcmp(op, "++") == 0 || strcmp(op, "--") == 0) {
        if (!ehNumerico(operando)) {
             fprintf(stderr, "Erro semantico na linha %d: Operador aritmetico unario '%s' aplicado a tipo nao-numerico.\n", no->linha, op);
             return TIPO_INDEFINIDO;
        }
        if (operando == TIPO_BYTE || operando == TIPO_SHORT || operando == TIPO_CHAR) {
            return TIPO_INT;
        }
        return operando;
    }

    return operando;
}

TipoDado inferirTipoExpressao(NoAST *expr) {
    if (!expr) return TIPO_INDEFINIDO;

    TipoDado tipo = TIPO_INDEFINIDO;

    switch (expr->tipo_no) {
        case NO_LITERAL_INT: tipo = TIPO_INT; break;
        case NO_LITERAL_DOUBLE: tipo = TIPO_DOUBLE; break;
        case NO_LITERAL_BOOL: tipo = TIPO_BOOLEAN; break;
        case NO_IDENTIFICADOR: tipo = tipoDoIdentificador(expr->valor.valor_str); break;
        case NO_OP_BINARIA: tipo = inferirOperacaoBinaria(expr); break;
        case NO_OP_UNARIA: tipo = inferirOperacaoUnaria(expr); break;
        default:
            for (int i = 0; i < expr->num_filhos; i++) {
                 inferirTipoExpressao(expr->filhos[i]);
            }
            break;
    }

    if (expr->tipo_no == NO_LITERAL_INT || expr->tipo_no == NO_LITERAL_DOUBLE || 
        expr->tipo_no == NO_LITERAL_BOOL || expr->tipo_no == NO_IDENTIFICADOR ||
        expr->tipo_no == NO_OP_BINARIA || expr->tipo_no == NO_OP_UNARIA) {
        expr->tipo_dado = tipo;
    }
    return tipo;
}

int analisarTiposAST(NoAST *raiz) {
    if (!raiz) return 0;
    
    int erros = 0;
    if (raiz->tipo_no == NO_OP_BINARIA || raiz->tipo_no == NO_OP_UNARIA || raiz->tipo_no == NO_IDENTIFICADOR) {
        if (raiz->tipo_dado == TIPO_INDEFINIDO) {
            erros++; 
        }
    }
    
    for (int i = 0; i < raiz->num_filhos; i++) {
        erros += analisarTiposAST(raiz->filhos[i]);
    }
    return erros;
}