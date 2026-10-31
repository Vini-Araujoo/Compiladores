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

/* =========================================================================
 * Passagem Semântica: Resolução de Escopos e Variáveis (Issue 8.3)
 * ========================================================================= */

static int erros_escopo = 0;
static int linha_corrente = 1;

static void verificarNoEscopo(NoAST *no);

static int obterLinha(const NoAST *no) {
    if (no && no->linha > 0) {
        linha_corrente = no->linha;
        return no->linha;
    }
    return linha_corrente;
}

static void verificarDeclaracao(NoAST *no) {
    if (!no) return;

    /* Container de múltiplas declarações (ex.: int a = 1, b, c = 3;) */
    if (no->valor.valor_str == NULL) {
        for (int i = 0; i < no->num_filhos; i++) {
            verificarDeclaracao(no->filhos[i]);
        }
        return;
    }

    char *nome = no->valor.valor_str;
    int linha = obterLinha(no);

    /* 1. Avalia/verifica a expressão inicializadora antes de registrar o símbolo */
    for (int i = 0; i < no->num_filhos; i++) {
        verificarNoEscopo(no->filhos[i]);
    }

    /* 2. Verifica se a variável já existe no escopo corrente (redeclaração proibida) */
    if (buscarSimboloEscopoAtual(nome) != NULL) {
        fprintf(stderr, "Erro semantico na linha %d: variavel '%s' ja declarada neste escopo.\n",
                linha, nome);
        erros_escopo++;
    } else {
        Modificadores mods = MODIFICADORES_PADRAO;
        inserirSimbolo(nome, no->tipo_dado, mods, linha);
        no->simbolo = buscarSimboloEscopoAtual(nome);
    }
}

static void verificarMetodoEscopo(NoAST *no) {
    if (!no) return;

    obterLinha(no);
    empilharEscopo();

    /* Filhos[0] = NO_PARAMETROS */
    if (no->num_filhos > 0 && no->filhos[0]) {
        NoAST *params = no->filhos[0];
        for (int i = 0; i < params->num_filhos; i++) {
            NoAST *p = params->filhos[i];
            if (p && (p->tipo_no == NO_PARAMETRO || p->tipo_no == NO_DECL_VARIAVEL) && p->valor.valor_str) {
                char *nome = p->valor.valor_str;
                int linha = obterLinha(p);
                if (buscarSimboloEscopoAtual(nome) != NULL) {
                    fprintf(stderr, "Erro semantico na linha %d: variavel '%s' ja declarada neste escopo.\n",
                            linha, nome);
                    erros_escopo++;
                } else {
                    Modificadores mods = MODIFICADORES_PADRAO;
                    inserirSimbolo(nome, p->tipo_dado, mods, linha);
                    p->simbolo = buscarSimboloEscopoAtual(nome);
                }
            }
        }
    }

    /* Filhos[1] = corpo do método (NO_BLOCO) */
    if (no->num_filhos > 1 && no->filhos[1]) {
        NoAST *corpo = no->filhos[1];
        if (corpo->tipo_no == NO_BLOCO) {
            /* O corpo do método compartilha o escopo dos parâmetros */
            for (int i = 0; i < corpo->num_filhos; i++) {
                verificarNoEscopo(corpo->filhos[i]);
            }
        } else {
            verificarNoEscopo(corpo);
        }
    }

    desempilharEscopo();
}

static void verificarClasseEscopo(NoAST *no) {
    if (!no) return;

    obterLinha(no);

    /* Passagem 1: cadastra todos os atributos/variáveis da classe no escopo de classe */
    for (int i = 0; i < no->num_filhos; i++) {
        NoAST *filho = no->filhos[i];
        if (filho && filho->tipo_no == NO_DECL_VARIAVEL) {
            verificarDeclaracao(filho);
        }
    }

    /* Passagem 2: analisa métodos e demais comandos da classe */
    for (int i = 0; i < no->num_filhos; i++) {
        NoAST *filho = no->filhos[i];
        if (filho && filho->tipo_no == NO_METODO) {
            verificarMetodoEscopo(filho);
        } else if (filho && filho->tipo_no != NO_DECL_VARIAVEL) {
            verificarNoEscopo(filho);
        }
    }
}

static void verificarNoEscopo(NoAST *no) {
    if (!no) return;

    obterLinha(no);

    switch (no->tipo_no) {
        case NO_PROGRAMA:
            for (int i = 0; i < no->num_filhos; i++) {
                verificarNoEscopo(no->filhos[i]);
            }
            break;

        case NO_CLASSE:
            verificarClasseEscopo(no);
            break;

        case NO_METODO:
            verificarMetodoEscopo(no);
            break;

        case NO_DECL_VARIAVEL:
            verificarDeclaracao(no);
            break;

        case NO_BLOCO:
            empilharEscopo();
            for (int i = 0; i < no->num_filhos; i++) {
                verificarNoEscopo(no->filhos[i]);
            }
            desempilharEscopo();
            break;

        case NO_FOR:
            empilharEscopo();
            /* inicializacao (pode conter declaracao de variavel de controle do loop) */
            if (no->num_filhos > 0 && no->filhos[0]) {
                verificarNoEscopo(no->filhos[0]);
            }
            /* condicao */
            if (no->num_filhos > 1 && no->filhos[1]) {
                verificarNoEscopo(no->filhos[1]);
            }
            /* incremento */
            if (no->num_filhos > 2 && no->filhos[2]) {
                verificarNoEscopo(no->filhos[2]);
            }
            /* corpo do loop */
            if (no->num_filhos > 3 && no->filhos[3]) {
                NoAST *corpo_for = no->filhos[3];
                if (corpo_for->tipo_no == NO_BLOCO) {
                    for (int i = 0; i < corpo_for->num_filhos; i++) {
                        verificarNoEscopo(corpo_for->filhos[i]);
                    }
                } else {
                    verificarNoEscopo(corpo_for);
                }
            }
            desempilharEscopo();
            break;

        case NO_IDENTIFICADOR: {
            char *nome = no->valor.valor_str;
            int linha = obterLinha(no);
            Simbolo *s = buscarSimbolo(nome);
            if (!s) {
                fprintf(stderr, "Erro semantico na linha %d: variavel '%s' nao declarada.\n",
                        linha, nome ? nome : "<identificador>");
                erros_escopo++;
            } else {
                no->simbolo = s;
                no->tipo_dado = s->tipo;
            }
            break;
        }

        case NO_ATRIBUICAO:
            for (int i = 0; i < no->num_filhos; i++) {
                verificarNoEscopo(no->filhos[i]);
            }
            if (no->num_filhos > 0 && no->filhos[0]) {
                if (no->filhos[0]->tipo_no == NO_IDENTIFICADOR && no->filhos[0]->simbolo) {
                    no->simbolo = no->filhos[0]->simbolo;
                    no->tipo_dado = no->filhos[0]->tipo_dado;
                }
            }
            break;

        case NO_INCREMENTO:
        case NO_DECREMENTO:
        case NO_INCREMENTO_ANTES:
        case NO_DECREMENTO_ANTES:
            for (int i = 0; i < no->num_filhos; i++) {
                verificarNoEscopo(no->filhos[i]);
            }
            if (no->num_filhos > 0 && no->filhos[0]) {
                if (no->filhos[0]->tipo_no == NO_IDENTIFICADOR && no->filhos[0]->simbolo) {
                    no->simbolo = no->filhos[0]->simbolo;
                    no->tipo_dado = no->filhos[0]->tipo_dado;
                }
            }
            break;

        default:
            for (int i = 0; i < no->num_filhos; i++) {
                verificarNoEscopo(no->filhos[i]);
            }
            break;
    }
}

int verificarEscopos(NoAST *raiz) {
    if (!raiz) return 1;

    erros_escopo = 0;
    linha_corrente = raiz->linha > 0 ? raiz->linha : 1;
    inicializarTabelaSimbolos();

    verificarNoEscopo(raiz);

    return (erros_escopo == 0) ? 1 : 0;
}
