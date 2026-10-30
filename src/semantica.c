#include "semantica.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AssinaturaMetodo *metodos;

static int tiposCompativeis(TipoDado esperado, TipoDado recebido) {
    if (esperado == recebido) return 1;
    /* Conversões numéricas widening aceitas pela linguagem Java. */
    if (esperado == TIPO_SHORT && recebido == TIPO_BYTE) return 1;
    if (esperado == TIPO_INT && (recebido == TIPO_BYTE || recebido == TIPO_SHORT)) return 1;
    if (esperado == TIPO_LONG && (recebido == TIPO_BYTE || recebido == TIPO_SHORT || recebido == TIPO_INT)) return 1;
    if (esperado == TIPO_FLOAT && (recebido == TIPO_BYTE || recebido == TIPO_SHORT || recebido == TIPO_INT || recebido == TIPO_LONG)) return 1;
    if (esperado == TIPO_DOUBLE && (recebido == TIPO_BYTE || recebido == TIPO_SHORT || recebido == TIPO_INT || recebido == TIPO_LONG || recebido == TIPO_FLOAT)) return 1;
    return 0;
}

void inicializarTabelaMetodos(void) { liberarTabelaMetodos(); }

void liberarTabelaMetodos(void) {
    while (metodos) {
        AssinaturaMetodo *atual = metodos;
        metodos = metodos->proximo;
        free(atual->nome);
        free(atual->tipos_parametros);
        free(atual);
    }
}

int registrarAssinaturaMetodo(const char *nome, TipoDado retorno,
                              const TipoDado *parametros, int quantidade) {
    if (!nome || quantidade < 0 || (quantidade > 0 && !parametros) || obterAssinaturaMetodo(nome)) return 0;
    AssinaturaMetodo *novo = calloc(1, sizeof(*novo));
    if (!novo) return 0;
    novo->nome = strdup(nome);
    if (!novo->nome) { free(novo); return 0; }
    if (quantidade > 0) {
        novo->tipos_parametros = malloc((size_t)quantidade * sizeof(*parametros));
        if (!novo->tipos_parametros) { free(novo->nome); free(novo); return 0; }
        memcpy(novo->tipos_parametros, parametros, (size_t)quantidade * sizeof(*parametros));
    }
    novo->tipo_retorno = retorno;
    novo->quantidade_parametros = quantidade;
    novo->proximo = metodos;
    metodos = novo;
    return 1;
}

const AssinaturaMetodo *obterAssinaturaMetodo(const char *nome) {
    for (AssinaturaMetodo *m = metodos; m; m = m->proximo)
        if (nome && strcmp(m->nome, nome) == 0) return m;
    return NULL;
}

TipoDado inferirTipoExpressao(const NoAST *no) {
    if (!no) return TIPO_VOID;
    switch (no->tipo_no) {
        case NO_LITERAL_INT: return TIPO_INT;
        case NO_LITERAL_FLOAT: return TIPO_FLOAT;
        case NO_LITERAL_DOUBLE: return TIPO_DOUBLE;
        case NO_LITERAL_STRING: return TIPO_STRING;
        case NO_LITERAL_CHAR: return TIPO_CHAR;
        case NO_LITERAL_BOOL: return TIPO_BOOLEAN;
        case NO_IDENTIFICADOR: {
            Simbolo *s = buscarSimbolo(no->valor.valor_str);
            return s ? s->tipo : TIPO_VOID;
        }
        case NO_EXPRESSAO:
            return no->num_filhos == 1 ? inferirTipoExpressao(no->filhos[0]) : TIPO_VOID;
        case NO_ACESSO_ARRAY:
            return TIPO_VOID; /* tipo do elemento depende da representação do símbolo */
        default: return no->tipo_dado;
    }
}

int validarAcessoArray(const char *nome, const NoAST *indice, int linha) {
    Simbolo *simbolo = buscarSimbolo((char *)nome);
    if (!simbolo || simbolo->tipo != TIPO_ARRAY) {
        fprintf(stderr, "Erro semantico na linha %d: '%s' nao foi declarado como array.\n", linha, nome ? nome : "<identificador>");
        return 0;
    }
    if (inferirTipoExpressao(indice) != TIPO_INT) {
        fprintf(stderr, "Erro semantico na linha %d: indice do array '%s' deve ser do tipo int.\n", linha, nome);
        return 0;
    }
    return 1;
}

int validarChamadaMetodo(const char *nome, const NoAST *argumentos, int linha) {
    const AssinaturaMetodo *assinatura = obterAssinaturaMetodo(nome);
    if (!assinatura) {
        fprintf(stderr, "Erro semantico na linha %d: metodo '%s' nao declarado.\n", linha, nome ? nome : "<metodo>");
        return 0;
    }
    int quantidade = argumentos ? argumentos->num_filhos : 0;
    if (quantidade != assinatura->quantidade_parametros) {
        fprintf(stderr, "Erro semantico na linha %d: metodo '%s' espera %d argumento(s), mas recebeu %d.\n", linha, nome, assinatura->quantidade_parametros, quantidade);
        return 0;
    }
    for (int i = 0; i < quantidade; i++) {
        TipoDado recebido = inferirTipoExpressao(argumentos->filhos[i]);
        TipoDado esperado = assinatura->tipos_parametros[i];
        if (!tiposCompativeis(esperado, recebido)) {
            fprintf(stderr, "Erro semantico na linha %d: argumento %d de '%s' possui tipo incompativel.\n", linha, i + 1, nome);
            return 0;
        }
    }
    return 1;
}
