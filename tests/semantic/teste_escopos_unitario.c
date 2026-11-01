#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/ast.h"
#include "../../src/tabela.h"
#include "../../src/semantica.h"

/*
 * Teste unitário em C puro para verificar a passagem semântica de resolução
 * de escopos e variáveis (Issue 8.3).
 */

static void teste_declaracao_e_shadowing_valido(void) {
    printf("  [1] Executando teste_declaracao_e_shadowing_valido...\n");

    /*
     * Monta a seguinte estrutura AST:
     * class Exemplo {
     *     int g;
     *     void metodo(int p) {
     *         int local;
     *         local = g + p;
     *         {
     *             int local; // shadowing permitido em bloco aninhado
     *             local = 10;
     *         }
     *     }
     * }
     */
    NoAST *classe = criarNo(NO_CLASSE, 1);
    classe->valor.valor_str = strdup("Exemplo");

    /* Atributo global g */
    NoAST *decl_g = criarNo(NO_DECL_VARIAVEL, 2);
    decl_g->tipo_dado = TIPO_INT;
    decl_g->valor.valor_str = strdup("g");
    adicionarFilho(classe, decl_g);

    /* Metodo */
    NoAST *metodo = criarNo(NO_METODO, 3);
    metodo->tipo_dado = TIPO_VOID;
    metodo->valor.valor_str = strdup("metodo");

    /* Parametros: int p */
    NoAST *params = criarNo(NO_PARAMETROS, 3);
    NoAST *param_p = criarNo(NO_PARAMETRO, 3);
    param_p->tipo_dado = TIPO_INT;
    param_p->valor.valor_str = strdup("p");
    adicionarFilho(params, param_p);
    adicionarFilho(metodo, params);

    /* Corpo do metodo */
    NoAST *corpo = criarNo(NO_BLOCO, 4);

    /* int local; */
    NoAST *decl_local = criarNo(NO_DECL_VARIAVEL, 4);
    decl_local->tipo_dado = TIPO_INT;
    decl_local->valor.valor_str = strdup("local");
    adicionarFilho(corpo, decl_local);

    /* local = g + p; */
    NoAST *atrib1 = criarNo(NO_ATRIBUICAO, 5);
    NoAST *id_local1 = criarNo(NO_IDENTIFICADOR, 5);
    id_local1->valor.valor_str = strdup("local");
    NoAST *soma = criarNo(NO_OP_BINARIA, 5);
    strncpy(soma->valor.operador, "+", sizeof(soma->valor.operador) - 1);
    NoAST *id_g = criarNo(NO_IDENTIFICADOR, 5);
    id_g->valor.valor_str = strdup("g");
    NoAST *id_p = criarNo(NO_IDENTIFICADOR, 5);
    id_p->valor.valor_str = strdup("p");
    adicionarFilho(soma, id_g);
    adicionarFilho(soma, id_p);
    adicionarFilho(atrib1, id_local1);
    adicionarFilho(atrib1, soma);
    adicionarFilho(corpo, atrib1);

    /* Bloco aninhado: { int local; local = 10; } */
    NoAST *bloco_interno = criarNo(NO_BLOCO, 6);
    NoAST *decl_local_shadow = criarNo(NO_DECL_VARIAVEL, 7);
    decl_local_shadow->tipo_dado = TIPO_INT;
    decl_local_shadow->valor.valor_str = strdup("local");
    adicionarFilho(bloco_interno, decl_local_shadow);

    NoAST *atrib2 = criarNo(NO_ATRIBUICAO, 8);
    NoAST *id_local2 = criarNo(NO_IDENTIFICADOR, 8);
    id_local2->valor.valor_str = strdup("local");
    NoAST *lit10 = criarNo(NO_LITERAL_INT, 8);
    lit10->valor.valor_int = 10;
    adicionarFilho(atrib2, id_local2);
    adicionarFilho(atrib2, lit10);
    adicionarFilho(bloco_interno, atrib2);

    adicionarFilho(corpo, bloco_interno);
    adicionarFilho(metodo, corpo);
    adicionarFilho(classe, metodo);

    int res = verificarEscopos(classe);
    assert(res == 1);

    /* Validar associação de ponteiros aos símbolos */
    assert(decl_g->simbolo != NULL);
    assert(strcmp(decl_g->simbolo->nome, "g") == 0);

    assert(param_p->simbolo != NULL);
    assert(strcmp(param_p->simbolo->nome, "p") == 0);

    assert(id_g->simbolo != NULL);
    assert(id_g->simbolo == decl_g->simbolo);

    assert(id_p->simbolo != NULL);
    assert(id_p->simbolo == param_p->simbolo);

    assert(id_local1->simbolo != NULL);
    assert(id_local1->simbolo == decl_local->simbolo);

    assert(id_local2->simbolo != NULL);
    assert(id_local2->simbolo == decl_local_shadow->simbolo);

    /* O símbolo sombreado no bloco interno é uma instância distinta do externo */
    assert(id_local1->simbolo != id_local2->simbolo);

    liberarAST(classe);
    liberarTabelaSimbolos();
    printf("    [OK] Declaracao e shadowing validados com sucesso.\n");
}

static void teste_variavel_nao_declarada(void) {
    printf("  [2] Executando teste_variavel_nao_declarada...\n");

    /*
     * class Invalido {
     *     void metodo() {
     *         x = 10; // 'x' nao foi declarada!
     *     }
     * }
     */
    NoAST *classe = criarNo(NO_CLASSE, 1);
    classe->valor.valor_str = strdup("Invalido");

    NoAST *metodo = criarNo(NO_METODO, 2);
    metodo->tipo_dado = TIPO_VOID;
    metodo->valor.valor_str = strdup("metodo");

    NoAST *params = criarNo(NO_PARAMETROS, 2);
    adicionarFilho(metodo, params);

    NoAST *corpo = criarNo(NO_BLOCO, 3);
    NoAST *atrib = criarNo(NO_ATRIBUICAO, 4);
    NoAST *id_x = criarNo(NO_IDENTIFICADOR, 4);
    id_x->valor.valor_str = strdup("x");
    NoAST *lit10 = criarNo(NO_LITERAL_INT, 4);
    lit10->valor.valor_int = 10;
    adicionarFilho(atrib, id_x);
    adicionarFilho(atrib, lit10);
    adicionarFilho(corpo, atrib);

    adicionarFilho(metodo, corpo);
    adicionarFilho(classe, metodo);

    int res = verificarEscopos(classe);
    assert(res == 0);
    assert(id_x->simbolo == NULL);

    liberarAST(classe);
    liberarTabelaSimbolos();
    printf("    [OK] Variavel nao declarada rejeitada corretamente.\n");
}

static void teste_redeclaracao_mesmo_escopo(void) {
    printf("  [3] Executando teste_redeclaracao_mesmo_escopo...\n");

    /*
     * class Duplicado {
     *     void metodo() {
     *         int a;
     *         int a; // Redeclaracao no mesmo bloco!
     *     }
     * }
     */
    NoAST *classe = criarNo(NO_CLASSE, 1);
    classe->valor.valor_str = strdup("Duplicado");

    NoAST *metodo = criarNo(NO_METODO, 2);
    metodo->tipo_dado = TIPO_VOID;
    metodo->valor.valor_str = strdup("metodo");

    NoAST *params = criarNo(NO_PARAMETROS, 2);
    adicionarFilho(metodo, params);

    NoAST *corpo = criarNo(NO_BLOCO, 3);

    NoAST *decl1 = criarNo(NO_DECL_VARIAVEL, 4);
    decl1->tipo_dado = TIPO_INT;
    decl1->valor.valor_str = strdup("a");
    adicionarFilho(corpo, decl1);

    NoAST *decl2 = criarNo(NO_DECL_VARIAVEL, 5);
    decl2->tipo_dado = TIPO_INT;
    decl2->valor.valor_str = strdup("a");
    adicionarFilho(corpo, decl2);

    adicionarFilho(metodo, corpo);
    adicionarFilho(classe, metodo);

    int res = verificarEscopos(classe);
    assert(res == 0);

    liberarAST(classe);
    liberarTabelaSimbolos();
    printf("    [OK] Redeclaracao no mesmo escopo rejeitada corretamente.\n");
}

static void teste_acesso_fora_do_escopo(void) {
    printf("  [4] Executando teste_acesso_fora_do_escopo...\n");

    /*
     * class EscopoFechado {
     *     void metodo() {
     *         {
     *             int temp;
     *         }
     *         temp = 5; // Invalido: 'temp' pertence ao bloco fechado!
     *     }
     * }
     */
    NoAST *classe = criarNo(NO_CLASSE, 1);
    classe->valor.valor_str = strdup("EscopoFechado");

    NoAST *metodo = criarNo(NO_METODO, 2);
    metodo->tipo_dado = TIPO_VOID;
    metodo->valor.valor_str = strdup("metodo");

    NoAST *params = criarNo(NO_PARAMETROS, 2);
    adicionarFilho(metodo, params);

    NoAST *corpo = criarNo(NO_BLOCO, 3);

    NoAST *bloco_interno = criarNo(NO_BLOCO, 4);
    NoAST *decl_temp = criarNo(NO_DECL_VARIAVEL, 5);
    decl_temp->tipo_dado = TIPO_INT;
    decl_temp->valor.valor_str = strdup("temp");
    adicionarFilho(bloco_interno, decl_temp);
    adicionarFilho(corpo, bloco_interno);

    NoAST *atrib = criarNo(NO_ATRIBUICAO, 7);
    NoAST *id_temp = criarNo(NO_IDENTIFICADOR, 7);
    id_temp->valor.valor_str = strdup("temp");
    NoAST *lit5 = criarNo(NO_LITERAL_INT, 7);
    lit5->valor.valor_int = 5;
    adicionarFilho(atrib, id_temp);
    adicionarFilho(atrib, lit5);
    adicionarFilho(corpo, atrib);

    adicionarFilho(metodo, corpo);
    adicionarFilho(classe, metodo);

    int res = verificarEscopos(classe);
    assert(res == 0);
    assert(id_temp->simbolo == NULL);

    liberarAST(classe);
    liberarTabelaSimbolos();
    printf("    [OK] Acesso a variavel fora do escopo rejeitado corretamente.\n");
}

static void teste_escopo_for(void) {
    printf("  [5] Executando teste_escopo_for...\n");

    /*
     * for (int i = 0; i < 10; i++) {
     *     i = i + 1;
     * }
     */
    NoAST *for_no = criarNo(NO_FOR, 1);

    NoAST *init = criarNo(NO_DECL_VARIAVEL, 1);
    init->tipo_dado = TIPO_INT;
    init->valor.valor_str = strdup("i");
    NoAST *lit0 = criarNo(NO_LITERAL_INT, 1);
    lit0->valor.valor_int = 0;
    adicionarFilho(init, lit0);

    NoAST *cond = criarNo(NO_OP_BINARIA, 1);
    strncpy(cond->valor.operador, "<", sizeof(cond->valor.operador) - 1);
    NoAST *id_cond = criarNo(NO_IDENTIFICADOR, 1);
    id_cond->valor.valor_str = strdup("i");
    NoAST *lit10 = criarNo(NO_LITERAL_INT, 1);
    lit10->valor.valor_int = 10;
    adicionarFilho(cond, id_cond);
    adicionarFilho(cond, lit10);

    NoAST *inc = criarNo(NO_INCREMENTO, 1);
    NoAST *id_inc = criarNo(NO_IDENTIFICADOR, 1);
    id_inc->valor.valor_str = strdup("i");
    adicionarFilho(inc, id_inc);

    NoAST *bloco = criarNo(NO_BLOCO, 2);
    NoAST *atrib = criarNo(NO_ATRIBUICAO, 2);
    NoAST *id_alvo = criarNo(NO_IDENTIFICADOR, 2);
    id_alvo->valor.valor_str = strdup("i");
    NoAST *soma = criarNo(NO_OP_BINARIA, 2);
    strncpy(soma->valor.operador, "+", sizeof(soma->valor.operador) - 1);
    NoAST *id_soma = criarNo(NO_IDENTIFICADOR, 2);
    id_soma->valor.valor_str = strdup("i");
    NoAST *lit1 = criarNo(NO_LITERAL_INT, 2);
    lit1->valor.valor_int = 1;
    adicionarFilho(soma, id_soma);
    adicionarFilho(soma, lit1);
    adicionarFilho(atrib, id_alvo);
    adicionarFilho(atrib, soma);
    adicionarFilho(bloco, atrib);

    adicionarFilho(for_no, init);
    adicionarFilho(for_no, cond);
    adicionarFilho(for_no, inc);
    adicionarFilho(for_no, bloco);

    int res = verificarEscopos(for_no);
    assert(res == 1);
    assert(id_cond->simbolo != NULL);
    assert(id_inc->simbolo != NULL);
    assert(id_alvo->simbolo != NULL);
    assert(id_soma->simbolo != NULL);
    assert(id_cond->simbolo == init->simbolo);

    liberarAST(for_no);
    liberarTabelaSimbolos();
    printf("    [OK] Escopo de comando for validado com sucesso.\n");
}

int main(void) {
    printf("=== Executando teste_escopos_unitario (Issue 8.3) ===\n");
    teste_declaracao_e_shadowing_valido();
    teste_variavel_nao_declarada();
    teste_redeclaracao_mesmo_escopo();
    teste_acesso_fora_do_escopo();
    teste_escopo_for();
    printf("teste_escopos_unitario passou com sucesso!\n");
    return 0;
}
