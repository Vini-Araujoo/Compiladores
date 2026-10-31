#include "semantica.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AssinaturaMetodo *metodos;

const char *nomeTipoDado(TipoDado tipo) {
    switch (tipo) {
        case TIPO_BYTE: return "byte";
        case TIPO_SHORT: return "short";
        case TIPO_INT: return "int";
        case TIPO_LONG: return "long";
        case TIPO_FLOAT: return "float";
        case TIPO_DOUBLE: return "double";
        case TIPO_CHAR: return "char";
        case TIPO_BOOLEAN: return "boolean";
        case TIPO_STRING: return "String";
        case TIPO_ARRAY: return "array";
        case TIPO_SCANNER: return "Scanner";
        case TIPO_VOID: return "void";
        default: return "desconhecido";
    }
}

int tiposCompativeis(TipoDado destino, TipoDado origem) {
    if (destino == origem) return 1;
    switch (origem) {
        case TIPO_BYTE:
            return (destino == TIPO_SHORT || destino == TIPO_INT || destino == TIPO_LONG ||
                    destino == TIPO_FLOAT || destino == TIPO_DOUBLE);
        case TIPO_SHORT:
            return (destino == TIPO_INT || destino == TIPO_LONG ||
                    destino == TIPO_FLOAT || destino == TIPO_DOUBLE);
        case TIPO_CHAR:
            return (destino == TIPO_INT || destino == TIPO_LONG ||
                    destino == TIPO_FLOAT || destino == TIPO_DOUBLE);
        case TIPO_INT:
            return (destino == TIPO_LONG || destino == TIPO_FLOAT || destino == TIPO_DOUBLE);
        case TIPO_LONG:
            return (destino == TIPO_FLOAT || destino == TIPO_DOUBLE);
        case TIPO_FLOAT:
            return (destino == TIPO_DOUBLE);
        default:
            return 0;
    }
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
            if (no->tipo_dado != TIPO_VOID) return no->tipo_dado;
            if (no->simbolo) return no->simbolo->tipo;
            Simbolo *s = buscarSimbolo(no->valor.valor_str);
            return s ? s->tipo : TIPO_VOID;
        }
        case NO_EXPRESSAO:
            if (no->tipo_dado != TIPO_VOID) return no->tipo_dado;
            return no->num_filhos == 1 ? inferirTipoExpressao(no->filhos[0]) : TIPO_VOID;
        case NO_OP_UNARIA: {
            if (no->tipo_dado != TIPO_VOID) return no->tipo_dado;
            if (strcmp(no->valor.operador, "!") == 0) return TIPO_BOOLEAN;
            if (no->num_filhos > 0 && no->filhos[0]) return inferirTipoExpressao(no->filhos[0]);
            return TIPO_VOID;
        }
        case NO_OP_BINARIA: {
            if (no->tipo_dado != TIPO_VOID) return no->tipo_dado;
            const char *op = no->valor.operador;
            if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
                strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 ||
                strcmp(op, ">") == 0 || strcmp(op, ">=") == 0 ||
                strcmp(op, "&&") == 0 || strcmp(op, "||") == 0) {
                return TIPO_BOOLEAN;
            }
            if (no->num_filhos >= 2 && no->filhos[0] && no->filhos[1]) {
                TipoDado t1 = inferirTipoExpressao(no->filhos[0]);
                TipoDado t2 = inferirTipoExpressao(no->filhos[1]);
                if (strcmp(op, "+") == 0 && (t1 == TIPO_STRING || t2 == TIPO_STRING)) {
                    return TIPO_STRING;
                }
                if (t1 == TIPO_DOUBLE || t2 == TIPO_DOUBLE) return TIPO_DOUBLE;
                if (t1 == TIPO_FLOAT || t2 == TIPO_FLOAT) return TIPO_FLOAT;
                if (t1 == TIPO_LONG || t2 == TIPO_LONG) return TIPO_LONG;
                if (t1 == TIPO_INT || t2 == TIPO_INT) return TIPO_INT;
                if (t1 != TIPO_VOID) return t1;
                return t2;
            }
            return TIPO_VOID;
        }
        case NO_CHAMADA_METODO: {
            if (no->tipo_dado != TIPO_VOID) return no->tipo_dado;
            const AssinaturaMetodo *m = obterAssinaturaMetodo(no->valor.valor_str);
            return m ? m->tipo_retorno : TIPO_VOID;
        }
        case NO_ACESSO_ARRAY:
            return no->tipo_dado;
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
            if (no->mods.eh_final || no->mods.is_final) {
                no->filhos[i]->mods = no->mods;
            }
            if (no->filhos[i]->tipo_dado == TIPO_VOID && no->tipo_dado != TIPO_VOID) {
                no->filhos[i]->tipo_dado = no->tipo_dado;
            }
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

    /* 2. Verifica compatibilidade de tipos na inicialização se houver expressão */
    if (no->num_filhos > 0 && no->filhos[0]) {
        TipoDado tipo_expr = inferirTipoExpressao(no->filhos[0]);
        if (tipo_expr != TIPO_VOID && !tiposCompativeis(no->tipo_dado, tipo_expr)) {
            fprintf(stderr, "Erro semantico na linha %d: tipos incompativeis na inicializacao de '%s'. Esperado '%s', recebido '%s'.\n",
                    linha, nome, nomeTipoDado(no->tipo_dado), nomeTipoDado(tipo_expr));
            erros_escopo++;
        }
    }

    /* 3. Verifica se a variável já existe no escopo corrente (redeclaração proibida) */
    if (buscarSimboloEscopoAtual(nome) != NULL) {
        fprintf(stderr, "Erro semantico na linha %d: variavel '%s' ja declarada neste escopo.\n",
                linha, nome);
        erros_escopo++;
    } else {
        Modificadores mods = no->mods;
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
                    Modificadores mods = p->mods;
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
                if (no->filhos[0]->tipo_no == NO_IDENTIFICADOR) {
                    Simbolo *s = no->filhos[0]->simbolo ? no->filhos[0]->simbolo : buscarSimbolo(no->filhos[0]->valor.valor_str);
                    if (s) {
                        no->simbolo = s;
                        no->tipo_dado = s->tipo;
                        int linha = obterLinha(no);
                        if (s->is_final || s->mods.eh_final) {
                            fprintf(stderr, "Erro semantico na linha %d: variavel final '%s' nao pode ser reatribuida.\n",
                                    linha, s->nome);
                            erros_escopo++;
                        }
                        if (no->num_filhos > 1 && no->filhos[1]) {
                            TipoDado tipo_origem = inferirTipoExpressao(no->filhos[1]);
                            TipoDado tipo_destino = s->tipo;
                            const char *op = no->valor.operador;
                            int compativel = 1;

                            if (op && strcmp(op, "+=") == 0 && tipo_destino == TIPO_STRING) {
                                compativel = 1;
                            } else if (tipo_origem != TIPO_VOID) {
                                compativel = tiposCompativeis(tipo_destino, tipo_origem);
                            }

                            if (!compativel) {
                                fprintf(stderr, "Erro semantico na linha %d: tipos incompativeis na atribuicao para '%s'. Esperado '%s', recebido '%s'.\n",
                                        linha, s->nome, nomeTipoDado(tipo_destino), nomeTipoDado(tipo_origem));
                                erros_escopo++;
                            }
                        }
                    }
                }
            }
            break;

        case NO_INCREMENTO:
        case NO_INCREMENTO_ANTES:
            for (int i = 0; i < no->num_filhos; i++) {
                verificarNoEscopo(no->filhos[i]);
            }
            if (no->num_filhos > 0 && no->filhos[0]) {
                if (no->filhos[0]->tipo_no == NO_IDENTIFICADOR) {
                    Simbolo *s = no->filhos[0]->simbolo ? no->filhos[0]->simbolo : buscarSimbolo(no->filhos[0]->valor.valor_str);
                    if (s) {
                        no->simbolo = s;
                        no->tipo_dado = s->tipo;
                        if (s->is_final || s->mods.eh_final) {
                            int linha = obterLinha(no);
                            fprintf(stderr, "Erro semantico na linha %d: variavel final '%s' nao pode ser incrementada.\n",
                                    linha, s->nome);
                            erros_escopo++;
                        }
                    }
                }
            }
            break;

        case NO_DECREMENTO:
        case NO_DECREMENTO_ANTES:
            for (int i = 0; i < no->num_filhos; i++) {
                verificarNoEscopo(no->filhos[i]);
            }
            if (no->num_filhos > 0 && no->filhos[0]) {
                if (no->filhos[0]->tipo_no == NO_IDENTIFICADOR) {
                    Simbolo *s = no->filhos[0]->simbolo ? no->filhos[0]->simbolo : buscarSimbolo(no->filhos[0]->valor.valor_str);
                    if (s) {
                        no->simbolo = s;
                        no->tipo_dado = s->tipo;
                        if (s->is_final || s->mods.eh_final) {
                            int linha = obterLinha(no);
                            fprintf(stderr, "Erro semantico na linha %d: variavel final '%s' nao pode ser decrementada.\n",
                                    linha, s->nome);
                            erros_escopo++;
                        }
                    }
                }
            }
            break;

        case NO_OP_UNARIA:
            for (int i = 0; i < no->num_filhos; i++) {
                verificarNoEscopo(no->filhos[i]);
            }
            if (no->num_filhos > 0 && no->filhos[0]) {
                if (strcmp(no->valor.operador, "++") == 0 || strcmp(no->valor.operador, "--") == 0) {
                    if (no->filhos[0]->tipo_no == NO_IDENTIFICADOR) {
                        Simbolo *s = no->filhos[0]->simbolo ? no->filhos[0]->simbolo : buscarSimbolo(no->filhos[0]->valor.valor_str);
                        if (s && (s->is_final || s->mods.eh_final)) {
                            int linha = obterLinha(no);
                            const char *acao = (strcmp(no->valor.operador, "++") == 0) ? "incrementada" : "decrementada";
                            fprintf(stderr, "Erro semantico na linha %d: variavel final '%s' nao pode ser %s.\n",
                                    linha, s->nome, acao);
                            erros_escopo++;
                        }
                    }
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
