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

static void teste_constante_final_valida(void) {
    printf("  [6] Executando teste_constante_final_valida...\n");

    /*
     * class ConstanteValida {
     *     void metodo() {
     *         final int LIMITE = 50;
     *         int v = LIMITE + 5;
     *     }
     * }
     */
    NoAST *classe = criarNo(NO_CLASSE, 1);
    classe->valor.valor_str = strdup("ConstanteValida");

    NoAST *metodo = criarNo(NO_METODO, 2);
    metodo->tipo_dado = TIPO_VOID;
    metodo->valor.valor_str = strdup("metodo");

    NoAST *params = criarNo(NO_PARAMETROS, 2);
    adicionarFilho(metodo, params);

    NoAST *corpo = criarNo(NO_BLOCO, 3);

    NoAST *decl_limite = criarNo(NO_DECL_VARIAVEL, 4);
    decl_limite->tipo_dado = TIPO_INT;
    decl_limite->mods.eh_final = 1;
    decl_limite->mods.is_final = 1;
    decl_limite->valor.valor_str = strdup("LIMITE");
    NoAST *lit50 = criarNo(NO_LITERAL_INT, 4);
    lit50->valor.valor_int = 50;
    adicionarFilho(decl_limite, lit50);
    adicionarFilho(corpo, decl_limite);

    NoAST *decl_v = criarNo(NO_DECL_VARIAVEL, 5);
    decl_v->tipo_dado = TIPO_INT;
    decl_v->valor.valor_str = strdup("v");
    NoAST *soma = criarNo(NO_OP_BINARIA, 5);
    strncpy(soma->valor.operador, "+", sizeof(soma->valor.operador) - 1);
    NoAST *id_limite = criarNo(NO_IDENTIFICADOR, 5);
    id_limite->valor.valor_str = strdup("LIMITE");
    NoAST *lit5 = criarNo(NO_LITERAL_INT, 5);
    lit5->valor.valor_int = 5;
    adicionarFilho(soma, id_limite);
    adicionarFilho(soma, lit5);
    adicionarFilho(decl_v, soma);
    adicionarFilho(corpo, decl_v);

    adicionarFilho(metodo, corpo);
    adicionarFilho(classe, metodo);

    int res = verificarEscopos(classe);
    assert(res == 1);
    assert(decl_limite->simbolo != NULL);
    assert(decl_limite->simbolo->is_final == 1);
    assert(id_limite->simbolo == decl_limite->simbolo);

    liberarAST(classe);
    liberarTabelaSimbolos();
    printf("    [OK] Constante final declarada, inicializada e consultada com sucesso.\n");
}

static void teste_mutacao_constante_final_invalida(void) {
    printf("  [7] Executando teste_mutacao_constante_final_invalida...\n");

    /* Caso A: Reatribuicao a constante final */
    {
        NoAST *classe = criarNo(NO_CLASSE, 1);
        classe->valor.valor_str = strdup("InvalidoReatrib");

        NoAST *metodo = criarNo(NO_METODO, 2);
        metodo->tipo_dado = TIPO_VOID;
        metodo->valor.valor_str = strdup("metodo");
        adicionarFilho(metodo, criarNo(NO_PARAMETROS, 2));

        NoAST *corpo = criarNo(NO_BLOCO, 3);
        NoAST *decl_c = criarNo(NO_DECL_VARIAVEL, 4);
        decl_c->tipo_dado = TIPO_INT;
        decl_c->mods.eh_final = 1;
        decl_c->mods.is_final = 1;
        decl_c->valor.valor_str = strdup("MAX");
        adicionarFilho(corpo, decl_c);

        NoAST *atrib = criarNo(NO_ATRIBUICAO, 5);
        NoAST *id_c = criarNo(NO_IDENTIFICADOR, 5);
        id_c->valor.valor_str = strdup("MAX");
        NoAST *lit20 = criarNo(NO_LITERAL_INT, 5);
        lit20->valor.valor_int = 20;
        adicionarFilho(atrib, id_c);
        adicionarFilho(atrib, lit20);
        adicionarFilho(corpo, atrib);

        adicionarFilho(metodo, corpo);
        adicionarFilho(classe, metodo);

        int res = verificarEscopos(classe);
        assert(res == 0);

        liberarAST(classe);
        liberarTabelaSimbolos();
    }

    /* Caso B: Incremento em constante final */
    {
        NoAST *classe = criarNo(NO_CLASSE, 1);
        classe->valor.valor_str = strdup("InvalidoInc");

        NoAST *metodo = criarNo(NO_METODO, 2);
        metodo->tipo_dado = TIPO_VOID;
        metodo->valor.valor_str = strdup("metodo");
        adicionarFilho(metodo, criarNo(NO_PARAMETROS, 2));

        NoAST *corpo = criarNo(NO_BLOCO, 3);
        NoAST *decl_c = criarNo(NO_DECL_VARIAVEL, 4);
        decl_c->tipo_dado = TIPO_INT;
        decl_c->mods.eh_final = 1;
        decl_c->mods.is_final = 1;
        decl_c->valor.valor_str = strdup("CONTADOR");
        adicionarFilho(corpo, decl_c);

        NoAST *inc = criarNo(NO_INCREMENTO, 5);
        NoAST *id_c = criarNo(NO_IDENTIFICADOR, 5);
        id_c->valor.valor_str = strdup("CONTADOR");
        adicionarFilho(inc, id_c);
        adicionarFilho(corpo, inc);

        adicionarFilho(metodo, corpo);
        adicionarFilho(classe, metodo);

        int res = verificarEscopos(classe);
        assert(res == 0);

        liberarAST(classe);
        liberarTabelaSimbolos();
    }

    printf("    [OK] Reatribuicao e incremento em constante final rejeitados com sucesso.\n");
}

static void teste_tipos_compativeis_matriz(void) {
    printf("  [8] Executando teste_tipos_compativeis_matriz (Issue 8.5)...\n");

    /* Widening conversions válidas do Java */
    assert(tiposCompativeis(TIPO_DOUBLE, TIPO_INT) == 1);
    assert(tiposCompativeis(TIPO_FLOAT, TIPO_INT) == 1);
    assert(tiposCompativeis(TIPO_LONG, TIPO_INT) == 1);
    assert(tiposCompativeis(TIPO_INT, TIPO_SHORT) == 1);
    assert(tiposCompativeis(TIPO_INT, TIPO_BYTE) == 1);
    assert(tiposCompativeis(TIPO_INT, TIPO_CHAR) == 1);
    assert(tiposCompativeis(TIPO_DOUBLE, TIPO_FLOAT) == 1);
    assert(tiposCompativeis(TIPO_DOUBLE, TIPO_LONG) == 1);
    assert(tiposCompativeis(TIPO_DOUBLE, TIPO_SHORT) == 1);
    assert(tiposCompativeis(TIPO_DOUBLE, TIPO_BYTE) == 1);
    assert(tiposCompativeis(TIPO_DOUBLE, TIPO_CHAR) == 1);
    assert(tiposCompativeis(TIPO_BOOLEAN, TIPO_BOOLEAN) == 1);
    assert(tiposCompativeis(TIPO_STRING, TIPO_STRING) == 1);

    /* Narrowing sem cast explícito (proibido) */
    assert(tiposCompativeis(TIPO_INT, TIPO_DOUBLE) == 0);
    assert(tiposCompativeis(TIPO_INT, TIPO_FLOAT) == 0);
    assert(tiposCompativeis(TIPO_INT, TIPO_LONG) == 0);
    assert(tiposCompativeis(TIPO_SHORT, TIPO_INT) == 0);
    assert(tiposCompativeis(TIPO_BYTE, TIPO_SHORT) == 0);
    assert(tiposCompativeis(TIPO_FLOAT, TIPO_DOUBLE) == 0);
    assert(tiposCompativeis(TIPO_CHAR, TIPO_INT) == 0);

    /* Tipos incompatíveis */
    assert(tiposCompativeis(TIPO_BOOLEAN, TIPO_INT) == 0);
    assert(tiposCompativeis(TIPO_INT, TIPO_BOOLEAN) == 0);
    assert(tiposCompativeis(TIPO_BOOLEAN, TIPO_STRING) == 0);
    assert(tiposCompativeis(TIPO_STRING, TIPO_BOOLEAN) == 0);
    assert(tiposCompativeis(TIPO_STRING, TIPO_INT) == 0);
    assert(tiposCompativeis(TIPO_INT, TIPO_STRING) == 0);

    /* Nomes dos tipos */
    assert(strcmp(nomeTipoDado(TIPO_INT), "int") == 0);
    assert(strcmp(nomeTipoDado(TIPO_DOUBLE), "double") == 0);
    assert(strcmp(nomeTipoDado(TIPO_BOOLEAN), "boolean") == 0);
    assert(strcmp(nomeTipoDado(TIPO_STRING), "String") == 0);

    printf("    [OK] Matriz de compatibilidade e conversoes widening validadas.\n");
}

static void teste_atribuicao_e_inicializacao_tipos(void) {
    printf("  [9] Executando teste_atribuicao_e_inicializacao_tipos (Issue 8.5)...\n");

    /* Caso 1: Atribuicao valida com coercao int -> double */
    {
        NoAST *classe = criarNo(NO_CLASSE, 1);
        classe->valor.valor_str = strdup("ValidoCoercao");

        NoAST *metodo = criarNo(NO_METODO, 2);
        metodo->tipo_dado = TIPO_VOID;
        metodo->valor.valor_str = strdup("metodo");
        adicionarFilho(metodo, criarNo(NO_PARAMETROS, 2));

        NoAST *corpo = criarNo(NO_BLOCO, 3);
        NoAST *decl_d = criarNo(NO_DECL_VARIAVEL, 4);
        decl_d->tipo_dado = TIPO_DOUBLE;
        decl_d->valor.valor_str = strdup("d");
        adicionarFilho(corpo, decl_d);

        NoAST *atrib = criarNo(NO_ATRIBUICAO, 5);
        NoAST *id_d = criarNo(NO_IDENTIFICADOR, 5);
        id_d->valor.valor_str = strdup("d");
        NoAST *lit42 = criarNo(NO_LITERAL_INT, 5);
        lit42->valor.valor_int = 42;
        adicionarFilho(atrib, id_d);
        adicionarFilho(atrib, lit42);
        adicionarFilho(corpo, atrib);

        adicionarFilho(metodo, corpo);
        adicionarFilho(classe, metodo);

        int res = verificarEscopos(classe);
        assert(res == 1);

        liberarAST(classe);
        liberarTabelaSimbolos();
    }

    /* Caso 2: Atribuicao invalida double -> int (narrowing) */
    {
        NoAST *classe = criarNo(NO_CLASSE, 1);
        classe->valor.valor_str = strdup("InvalidoNarrowing");

        NoAST *metodo = criarNo(NO_METODO, 2);
        metodo->tipo_dado = TIPO_VOID;
        metodo->valor.valor_str = strdup("metodo");
        adicionarFilho(metodo, criarNo(NO_PARAMETROS, 2));

        NoAST *corpo = criarNo(NO_BLOCO, 3);
        NoAST *decl_a = criarNo(NO_DECL_VARIAVEL, 4);
        decl_a->tipo_dado = TIPO_INT;
        decl_a->valor.valor_str = strdup("a");
        adicionarFilho(corpo, decl_a);

        NoAST *atrib = criarNo(NO_ATRIBUICAO, 5);
        NoAST *id_a = criarNo(NO_IDENTIFICADOR, 5);
        id_a->valor.valor_str = strdup("a");
        NoAST *litDouble = criarNo(NO_LITERAL_DOUBLE, 5);
        litDouble->valor.valor_double = 3.14;
        adicionarFilho(atrib, id_a);
        adicionarFilho(atrib, litDouble);
        adicionarFilho(corpo, atrib);

        adicionarFilho(metodo, corpo);
        adicionarFilho(classe, metodo);

        int res = verificarEscopos(classe);
        assert(res == 0);

        liberarAST(classe);
        liberarTabelaSimbolos();
    }

    /* Caso 3: Inicializacao invalida String -> boolean */
    {
        NoAST *classe = criarNo(NO_CLASSE, 1);
        classe->valor.valor_str = strdup("InvalidoInit");

        NoAST *metodo = criarNo(NO_METODO, 2);
        metodo->tipo_dado = TIPO_VOID;
        metodo->valor.valor_str = strdup("metodo");
        adicionarFilho(metodo, criarNo(NO_PARAMETROS, 2));

        NoAST *corpo = criarNo(NO_BLOCO, 3);
        NoAST *decl_flag = criarNo(NO_DECL_VARIAVEL, 4);
        decl_flag->tipo_dado = TIPO_BOOLEAN;
        decl_flag->valor.valor_str = strdup("flag");
        NoAST *litStr = criarNo(NO_LITERAL_STRING, 4);
        litStr->valor.valor_str = strdup("texto");
        adicionarFilho(decl_flag, litStr);
        adicionarFilho(corpo, decl_flag);

        adicionarFilho(metodo, corpo);
        adicionarFilho(classe, metodo);

        int res = verificarEscopos(classe);
        assert(res == 0);

        liberarAST(classe);
        liberarTabelaSimbolos();
    }

    printf("    [OK] Verificacao semantica de atribuicoes e inicializacoes validada.\n");
}

int main(void) {
    printf("=== Executando teste_escopos_unitario (Issues 8.3, 8.4 e 8.5) ===\n");
    teste_declaracao_e_shadowing_valido();
    teste_variavel_nao_declarada();
    teste_redeclaracao_mesmo_escopo();
    teste_acesso_fora_do_escopo();
    teste_escopo_for();
    teste_constante_final_valida();
    teste_mutacao_constante_final_invalida();
    teste_tipos_compativeis_matriz();
    teste_atribuicao_e_inicializacao_tipos();
    printf("teste_escopos_unitario passou com sucesso!\n");
    return 0;
}
