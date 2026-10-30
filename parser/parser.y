%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/ast.h"

int yylex(void);
void yyerror(const char *mensagem);
extern int yylineno;

NoAST *arvore = NULL;

// Armazena o nome da classe no proprio no da classe.
static NoAST *finalizar_declaracao_classe(NoAST *classe, char *nome) {
    classe->valor.valor_str = nome;
    return classe;
}

/*
membro_classe:
      declaracao_variavel { $$ = criarNo(NO_VAR_DECL, 0); }
      | declaracao_metodo { $$ = $1; }
    ;

*/
static NoAST *cria_no_instancia_id(TipoNoAST tipo, TipoDado dadoRetorno, char *id) {
    NoAST *no = criarNo(tipo, yylineno);
    no->tipo_dado = dadoRetorno;
    no->valor.valor_str = id;
    return no;
}

/* NO_OP_BINARIA: filhos[0] = operando esquerdo, filhos[1] = operando direito. */
static NoAST *cria_no_op_binaria(const char *operador, NoAST *esquerda, NoAST *direita) {
    NoAST *no = criarNo(NO_OP_BINARIA, esquerda->linha);
    strncpy(no->valor.operador, operador, sizeof(no->valor.operador) - 1);
    no->valor.operador[sizeof(no->valor.operador) - 1] = '\0';
    adicionarFilho(no, esquerda);
    adicionarFilho(no, direita);
    return no;
}

/*
 * NO_OP_UNARIA: filhos[0] = operando; valor.operador = "!", "-", "++" ou "--";
 * posfixo indica a direcao (1 = x++ / x--, 0 = !x, -x, ++x, --x).
 */
static NoAST *cria_no_op_unaria(const char *operador, NoAST *operando, int posfixo) {
    NoAST *no = criarNo(NO_OP_UNARIA, operando->linha);
    strncpy(no->valor.operador, operador, sizeof(no->valor.operador) - 1);
    no->valor.operador[sizeof(no->valor.operador) - 1] = '\0';
    no->posfixo = posfixo;
    adicionarFilho(no, operando);
    return no;
}

static NoAST *cria_no_literal_int(long valor) {
    NoAST *no = criarNo(NO_LITERAL_INT, yylineno);
    no->valor.valor_int = valor;
    return no;
}

static NoAST *cria_no_literal_double(double valor) {
    NoAST *no = criarNo(NO_LITERAL_DOUBLE, yylineno);
    no->valor.valor_double = valor;
    return no;
}

/* NO_LITERAL_BOOL: valor.valor_int = 1 (true) ou 0 (false). */
static NoAST *cria_no_literal_bool(int valor) {
    NoAST *no = criarNo(NO_LITERAL_BOOL, yylineno);
    no->valor.valor_int = valor;
    return no;
}

/*
 * Stub (Issue 9.1): no generico para as formas de expressao que pertencem a
 * outras issues (chamadas de metodo e leitura do Scanner).
 */
static NoAST *cria_no_expressao_generica(NoAST *filho1, NoAST *filho2) {
    NoAST *no = criarNo(NO_EXPRESSAO, yylineno);
    if (filho1) {
        adicionarFilho(no, filho1);
    }
    if (filho2) {
        adicionarFilho(no, filho2);
    }
    return no;
}
%}

// primitivos
%token TOKEN_BYTE
%token TOKEN_SHORT
%token <text> ID
%token <inteiro> NUMERO
%token <real> NUMERO_DOUBLE
%token TOKEN_INT
%token TOKEN_LONG
%token TOKEN_BOOLEAN
%token TOKEN_CHAR
%token TOKEN_FLOAT
%token TOKEN_DOUBLE
%token TOKEN_TRUE
%token TOKEN_FALSE

// operadores
%token TOKEN_ATRIB
%token TOKEN_IGUAL
%token TOKEN_DIFERENTE
%token TOKEN_MENOR_IGUAL
%token TOKEN_MAIOR_IGUAL
%token TOKEN_MENOR
%token TOKEN_MAIOR
%token TOKEN_MAIS
%token TOKEN_MENOS
%token TOKEN_VEZES
%token TOKEN_DIV
%token TOKEN_MOD
%token TOKEN_E
%token TOKEN_OU

// delimitadores
%token TOKEN_PTOVIRG  // ;
%token TOKEN_VIRGULA
%token TOKEN_ABRE_PAR   // (
%token TOKEN_FECHA_PAR  // )
%token TOKEN_ABRE_CHAVE
%token TOKEN_FECHA_CHAVE

// condicionais e repetição
%token TOKEN_IF
%token TOKEN_ELSE
%token TOKEN_SWITCH
%token TOKEN_CASE
%token TOKEN_DEFAULT
%token TOKEN_BREAK
%token TOKEN_DO
%token TOKEN_WHILE
%token TOKEN_FOR
%token TOKEN_RETURN

// identificação de alguns tokens de entrada em java
%token TOKEN_STRING_TIPO
%token TOKEN_SYSTEM  // System
%token TOKEN_OUT // out
%token TOKEN_PRINTLN
%token TOKEN_PRINT
%token TOKEN_SCANNER  // Scanner

// modificadores e declaração de classe
%token TOKEN_PUBLIC
%token TOKEN_PRIVATE
%token TOKEN_PROTECTED
%token TOKEN_STATIC
%token TOKEN_CLASS
%token TOKEN_VOID   // retorno vazio
%token TOKEN_FINAL  // const em java

// identificação de tokens para arrays e acesso
%token TOKEN_ABRE_COLCHETE
%token TOKEN_FECHA_COLCHETE
%token TOKEN_PONTO

// operadores adicionais
%token TOKEN_SOMA_ATRIB
%token TOKEN_SUB_ATRIB
%token TOKEN_INCREMENTO
%token TOKEN_DECREMENTO
%token TOKEN_NEGACAO

// instancia e entrada
%token TOKEN_NEW
%token TOKEN_NEXT
%token TOKEN_NEXT_LINE
%token TOKEN_NEXT_INT
%token TOKEN_NEXT_DOUBLE
%token TOKEN_NEXT_FLOAT
%token TOKEN_NEXT_LONG
%token TOKEN_NEXT_SHORT
%token TOKEN_NEXT_BYTE
%token TOKEN_NEXT_BOOLEAN
%token TOKEN_IN
%token TOKEN_DOIS_PONTOS

// Precedencia e associatividade das expressoes, da menor para a maior.
/*
 * ID e ';' (menor e maior precedencia desta lista) resolvem a ambiguidade entre
 * os comandos "i++;" / "++i;" de atribuicao e o unario ++/-- de expressao:
 * como comando isolado vale a regra de atribuicao; dentro de uma expressao
 * (ex.: x = i++;) vale o NO_OP_UNARIA.
 */
%precedence ID
%left   TOKEN_OU
%left   TOKEN_E
%left   TOKEN_IGUAL TOKEN_DIFERENTE
%left   TOKEN_MENOR TOKEN_MAIOR TOKEN_MENOR_IGUAL TOKEN_MAIOR_IGUAL
%left   TOKEN_MAIS TOKEN_MENOS
%left   TOKEN_VEZES TOKEN_DIV TOKEN_MOD
%precedence TOKEN_NEGACAO UMINUS
%precedence TOKEN_INCREMENTO TOKEN_DECREMENTO
%precedence TOKEN_PTOVIRG

%start programa

%precedence LOWER_THAN_ELSE
%precedence TOKEN_ELSE

%code requires {
    #include "../src/ast.h"
}

%union {
    NoAST *no;
    char *text;
    TipoDado tipo_dado;
    long inteiro;
    double real;
}

%destructor { free($$); } <text>

%type <no> programa declaracao_classe corpo_classe membro_classe
%type <no> bloco comando lista_comandos condicional condicao comando_if
%type <no> declaracao_local atribuicao comando_io repeticao comando_salto
%type <no> expressao comando_return comando_switch casos_switch case_switch
%type <no> default_switch declaracao_metodo comando_print argumento_opcional
%type <no> acesso_array expressao_sintatica leitura_scanner chamada_metodo
%type <no> instanciacao
%type <no> atribuicao_for condicao_for incremento_for
%type <no> comando_for comando_do_while comando_while
%type <tipo_dado> tipo_base
%type <tipo_dado> tipo
%type <no> lista_parametros parametros_com_virgula parametro
%%

/* Estrutura do programa e blocos */

programa:
      declaracao_classe 
        {
            $$ = $1;
            arvore = $$;
        }
    //| lista_instrucoes_io
    //| %empty
    ;

declaracao_classe:
        TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
        { $$ = finalizar_declaracao_classe($4, $2); }
      | TOKEN_FINAL TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
        { $$ = finalizar_declaracao_classe($5, $3); }
      | TOKEN_PUBLIC TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
        { $$ = finalizar_declaracao_classe($5, $3); }
      | TOKEN_PUBLIC TOKEN_FINAL TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
        { $$ = finalizar_declaracao_classe($6, $4); }
    ;

corpo_classe:
      %empty { $$ = criarNo(NO_CLASSE, 0); }
      | corpo_classe membro_classe 
        {
            adicionarFilho($1, $2);
            $$ = $1;
        }
    ;

membro_classe:
      declaracao_variavel { $$ = criarNo(NO_VAR_DECL, 0); }
      | declaracao_metodo { $$ = $1; }
    ;


declaracao_metodo:
  modificadores tipo ID TOKEN_ABRE_PAR lista_parametros TOKEN_FECHA_PAR bloco
    {
        //free($3);
        //$$ = criarNo(NO_METODO, 0);
        //adicionarFilho($$, $7);

        $$ = cria_no_instancia_id(NO_METODO, $2, $3);
        adicionarFilho($$, $5);
        adicionarFilho($$, $7);
    }
    | modificadores TOKEN_VOID ID TOKEN_ABRE_PAR lista_parametros TOKEN_FECHA_PAR bloco
    {
        //free($3);
        //$$ = criarNo(NO_METODO, 0);
        //adicionarFilho($$, $7);
        
        $$ = cria_no_instancia_id(NO_METODO, TIPO_VOID, $3);
        adicionarFilho($$, $5);
        adicionarFilho($$, $7);
    }
    ;

lista_parametros:
        %empty
        {
            $$ = criarNo(NO_PARAMETROS, yylineno);
        }
      | parametros_com_virgula
        {
            $$ = $1;
        }
    ;

parametros_com_virgula:
        parametro
        {
            $$ = criarNo(NO_PARAMETROS, yylineno);
            adicionarFilho($$, $1);
        }
      | parametros_com_virgula TOKEN_VIRGULA parametro
        {
            adicionarFilho($1, $3);
            $$ = $1;
        }
    ;

parametro:
        tipo ID
        {
            $$ = cria_no_instancia_id(NO_PARAMETRO, $1, $2);
        }
      | tipo TOKEN_ABRE_COLCHETE TOKEN_FECHA_COLCHETE ID
        {
            $$ = cria_no_instancia_id(NO_PARAMETRO, TIPO_ARRAY, $4);
        }
    ;

modificadores:
        %empty
      | modificadores modificador
    ; 
    
modificador:
        TOKEN_PUBLIC
      | TOKEN_PRIVATE
      | TOKEN_PROTECTED
      | TOKEN_STATIC
      | TOKEN_FINAL
    ;
tipo:
        TOKEN_DOUBLE
        {$$ = TIPO_DOUBLE; }
      | TOKEN_INT
        {$$ = TIPO_INT; }
      | TOKEN_STRING_TIPO
        {$$ = TIPO_STRING; }
      | TOKEN_LONG
        {$$ = TIPO_LONG; }
      | TOKEN_BOOLEAN
        {$$ = TIPO_BOOLEAN; }
      | TOKEN_SHORT
        {$$ = TIPO_SHORT; }
      | TOKEN_CHAR
        {$$ = TIPO_CHAR; }
      | TOKEN_FLOAT
        {$$ = TIPO_FLOAT; }
      | TOKEN_SCANNER
        {$$ = TIPO_SCANNER; }
    ;


//Atributos da classe (STUB da Issue 1)
declaracao_variavel:
        modificadores tipo lista_declaracoes TOKEN_PTOVIRG
      ;

lista_declaracoes:
        item_declaracao
      | lista_declaracoes TOKEN_VIRGULA item_declaracao
      ;

item_declaracao:
        ID
      | ID TOKEN_ATRIB expressao
      ;

modificador_variavel:
        %empty
      | TOKEN_FINAL
      ;

// (STUB da Issue 3 apenas abre e fecha chaves por enquanto)
bloco:
    TOKEN_ABRE_CHAVE lista_comandos TOKEN_FECHA_CHAVE  { $$ = $2; }
    ;

lista_comandos:
      %empty
        {
            $$ = criarNo(NO_BLOCO, 0); 
        }
    | lista_comandos comando 
    { 
        adicionarFilho($1, $2); 
        $$ = $1;
    }
    ;

comando:
  declaracao_local { $$ = $1; }
    | atribuicao { $$ = $1; }
    | comando_io { $$ = $1; }
    | condicional { $$ = $1; }
    | repeticao { $$ = $1; }
    | comando_salto { $$ = $1; }
    | bloco { $$ = $1; }
    ;

/* Expressões e comandos básicos */

expressao:
      expressao_sintatica { $$ = $1; }
    ;

/*
 * Issues 9.1 e 9.2: aritmetica, relacionais, logicos e unarios constroem a
 * AST; a hierarquia vem das declaracoes de precedencia. Parenteses devolvem o
 * no interno. Chamadas de metodo e Scanner ainda sao stubs genericos.
 */
expressao_sintatica:
      expressao_sintatica TOKEN_OU expressao_sintatica
        { $$ = cria_no_op_binaria("||", $1, $3); }
    | expressao_sintatica TOKEN_E expressao_sintatica
        { $$ = cria_no_op_binaria("&&", $1, $3); }
    | expressao_sintatica TOKEN_MAIS expressao_sintatica
        { $$ = cria_no_op_binaria("+", $1, $3); }
    | expressao_sintatica TOKEN_MENOS expressao_sintatica
        { $$ = cria_no_op_binaria("-", $1, $3); }
    | expressao_sintatica TOKEN_VEZES expressao_sintatica
        { $$ = cria_no_op_binaria("*", $1, $3); }
    | expressao_sintatica TOKEN_DIV expressao_sintatica
        { $$ = cria_no_op_binaria("/", $1, $3); }
    | expressao_sintatica TOKEN_MOD expressao_sintatica
        { $$ = cria_no_op_binaria("%", $1, $3); }
    | expressao_sintatica TOKEN_MENOR expressao_sintatica
        { $$ = cria_no_op_binaria("<", $1, $3); }
    | expressao_sintatica TOKEN_MAIOR expressao_sintatica
        { $$ = cria_no_op_binaria(">", $1, $3); }
    | expressao_sintatica TOKEN_MENOR_IGUAL expressao_sintatica
        { $$ = cria_no_op_binaria("<=", $1, $3); }
    | expressao_sintatica TOKEN_MAIOR_IGUAL expressao_sintatica
        { $$ = cria_no_op_binaria(">=", $1, $3); }
    | expressao_sintatica TOKEN_IGUAL expressao_sintatica
        { $$ = cria_no_op_binaria("==", $1, $3); }
    | expressao_sintatica TOKEN_DIFERENTE expressao_sintatica
        { $$ = cria_no_op_binaria("!=", $1, $3); }
    | TOKEN_NEGACAO expressao_sintatica
        { $$ = cria_no_op_unaria("!", $2, 0); }
    | TOKEN_MENOS expressao_sintatica %prec UMINUS
        { $$ = cria_no_op_unaria("-", $2, 0); }
    | TOKEN_INCREMENTO expressao_sintatica %prec UMINUS
        { $$ = cria_no_op_unaria("++", $2, 0); }
    | TOKEN_DECREMENTO expressao_sintatica %prec UMINUS
        { $$ = cria_no_op_unaria("--", $2, 0); }
    | expressao_sintatica TOKEN_INCREMENTO
        { $$ = cria_no_op_unaria("++", $1, 1); }
    | expressao_sintatica TOKEN_DECREMENTO
        { $$ = cria_no_op_unaria("--", $1, 1); }
    | TOKEN_ABRE_PAR expressao_sintatica TOKEN_FECHA_PAR
        { $$ = $2; }
    | ID
        { $$ = cria_no_instancia_id(NO_IDENTIFICADOR, TIPO_VOID, $1); }
    | NUMERO
        { $$ = cria_no_literal_int($1); }
    | NUMERO_DOUBLE
        { $$ = cria_no_literal_double($1); }
    | TOKEN_TRUE
        { $$ = cria_no_literal_bool(1); }
    | TOKEN_FALSE
        { $$ = cria_no_literal_bool(0); }
    | acesso_array { $$ = $1; }
    | leitura_scanner { $$ = $1; }
    | chamada_metodo { $$ = $1; }
    | instanciacao { $$ = $1; }
    ;

declaracao_local:
      modificador_variavel tipo lista_declaracoes TOKEN_PTOVIRG
        { $$ = criarNo(NO_VAR_DECL, 0); }
    ;

atribuicao:
      ID TOKEN_ATRIB expressao TOKEN_PTOVIRG
        {
            $$ = criarNo(NO_ATRIBUICAO, 0);
            NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
            id->valor.valor_str = $1;
            adicionarFilho($$, id);
            adicionarFilho($$, $3);
        }
    | acesso_array TOKEN_ATRIB expressao TOKEN_PTOVIRG
        {
            $$ = criarNo(NO_ATRIBUICAO, 0);
            adicionarFilho($$, $1);
            adicionarFilho($$, $3);
        }
    | TOKEN_INCREMENTO ID TOKEN_PTOVIRG
        {
            $$ = criarNo(NO_INCREMENTO_ANTES, 0);
            NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
            id->valor.valor_str = $2;
            adicionarFilho($$, id);
        }
    | TOKEN_DECREMENTO ID TOKEN_PTOVIRG
        {
            $$ = criarNo(NO_DECREMENTO_ANTES, 0);
            NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
            id->valor.valor_str = $2;
            adicionarFilho($$, id);
        }
    | ID TOKEN_INCREMENTO TOKEN_PTOVIRG
        {
            $$ = criarNo(NO_INCREMENTO, 0);
            NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
            id->valor.valor_str = $1;
            adicionarFilho($$, id);
        }
    | ID TOKEN_DECREMENTO TOKEN_PTOVIRG
        {
            $$ = criarNo(NO_DECREMENTO, 0);
            NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
            id->valor.valor_str = $1;
            adicionarFilho($$, id);
        }
    | ID TOKEN_SOMA_ATRIB expressao TOKEN_PTOVIRG
        {
            $$ = criarNo(NO_ATRIBUICAO, 0);
            NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
            id->valor.valor_str = $1;
            adicionarFilho($$, id);
            adicionarFilho($$, $3);
        }
    | ID TOKEN_SUB_ATRIB expressao TOKEN_PTOVIRG
        {
            $$ = criarNo(NO_ATRIBUICAO, 0);
            NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
            id->valor.valor_str = $1;
            adicionarFilho($$, id);
            adicionarFilho($$, $3);
        }
    ;

/* Entrada, saída, instanciação, arrays e chamadas qualificadas */

comando_io:
      comando_print { $$ = $1; }
    | expressao TOKEN_PTOVIRG { $$ = $1; }
    ;

comando_print:
      TOKEN_SYSTEM TOKEN_PONTO TOKEN_OUT TOKEN_PONTO TOKEN_PRINTLN
      TOKEN_ABRE_PAR argumento_opcional TOKEN_FECHA_PAR TOKEN_PTOVIRG
        {
            $$ = criarNo(NO_PRINTLN, 0);
            adicionarFilho($$, $7);
        }
    | TOKEN_SYSTEM TOKEN_PONTO TOKEN_OUT TOKEN_PONTO TOKEN_PRINT
      TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR TOKEN_PTOVIRG
        {
            $$ = criarNo(NO_PRINT, 0);
            adicionarFilho($$, $7);
        }
    ;

argumento_opcional:
      %empty { $$ = NULL; }
    | expressao { $$ = $1; }
    ;

instanciacao:
      TOKEN_NEW TOKEN_SCANNER TOKEN_ABRE_PAR TOKEN_SYSTEM TOKEN_PONTO
      TOKEN_IN TOKEN_FECHA_PAR
        {
            $$ = criarNo(NO_SCANNER, yylineno);
            $$->tipo_dado = TIPO_SCANNER;
        }
    | TOKEN_NEW tipo_base TOKEN_ABRE_COLCHETE expressao TOKEN_FECHA_COLCHETE
        {
            $$ = criarNo(NO_INSTANCIACAO_ARRAY, yylineno);
            $$->tipo_dado = $2;
            adicionarFilho($$, $4);
        }
    ;

tipo_base:
      TOKEN_INT         { $$ = TIPO_INT; }
    | TOKEN_DOUBLE      { $$ = TIPO_DOUBLE; }
    | TOKEN_STRING_TIPO { $$ = TIPO_STRING; }
    ;

acesso_array:
      ID TOKEN_ABRE_COLCHETE expressao TOKEN_FECHA_COLCHETE
        {
            $$ = criarNo(NO_ACESSO_ARRAY, yylineno);
            NoAST *id = criarNo(NO_IDENTIFICADOR, yylineno);
            id->valor.valor_str = $1;
            adicionarFilho($$, id);
            adicionarFilho($$, $3);
        }
    ;

leitura_scanner:
      ID TOKEN_PONTO metodo_scanner TOKEN_ABRE_PAR TOKEN_FECHA_PAR
        { free($1); $$ = cria_no_expressao_generica(NULL, NULL); }
    ;

metodo_scanner:
      TOKEN_NEXT
    | TOKEN_NEXT_LINE
    | TOKEN_NEXT_INT
    | TOKEN_NEXT_DOUBLE
    | TOKEN_NEXT_FLOAT
    | TOKEN_NEXT_LONG
    | TOKEN_NEXT_SHORT
    | TOKEN_NEXT_BYTE
    | TOKEN_NEXT_BOOLEAN
    ;

chamada_metodo:
      ID TOKEN_ABRE_PAR argumentos_opcionais TOKEN_FECHA_PAR
        { free($1); $$ = cria_no_expressao_generica(NULL, NULL); }
    | ID TOKEN_PONTO ID TOKEN_ABRE_PAR argumentos_opcionais TOKEN_FECHA_PAR
        { free($1); free($3); $$ = cria_no_expressao_generica(NULL, NULL); }
    ;

argumentos_opcionais:
      %empty
    | lista_argumentos
    ;

lista_argumentos:
      expressao
    | lista_argumentos TOKEN_VIRGULA expressao
    ;

comando_return:
      TOKEN_RETURN TOKEN_PTOVIRG { $$ = criarNo(NO_RETURN, 0); }
    | TOKEN_RETURN expressao TOKEN_PTOVIRG
    {
        $$ = criarNo(NO_RETURN, 0);
        adicionarFilho($$, $2);
    }
    ;

comando_salto:
      TOKEN_BREAK TOKEN_PTOVIRG { $$ = criarNo(NO_BREAK, 0); }
    | comando_return
    ;

/* Condicoes usam a mesma precedencia da expressao booleana. */
/* Stub enquanto a expressao não retorna os nós */
condicao:
  expressao
    {
        $$ = criarNo(NO_CONDICAO, 0);
    }
    ;

/* Condicionais */

comando_if:
      TOKEN_IF TOKEN_ABRE_PAR condicao TOKEN_FECHA_PAR comando
        %prec LOWER_THAN_ELSE
        {
            $$ = criarNo(NO_IF, 0);
            adicionarFilho($$, $3);
            adicionarFilho($$, $5);
        }
    | TOKEN_IF TOKEN_ABRE_PAR condicao TOKEN_FECHA_PAR comando
      TOKEN_ELSE comando
        {
            $$ = criarNo(NO_IF, 0);
            adicionarFilho($$, $3);
            adicionarFilho($$, $5);
            adicionarFilho($$, $7);
        }
    ;

comando_switch:
    TOKEN_SWITCH TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR
    TOKEN_ABRE_CHAVE casos_switch default_switch TOKEN_FECHA_CHAVE
    {
        $$ = criarNo(NO_SWITCH, 0);
        adicionarFilho($$, $3);
        adicionarFilho($$, $6);
        adicionarFilho($$, $7);
    }
    ;

casos_switch:
      %empty { $$ = criarNo(NO_CASES, 0); }
    | casos_switch case_switch
    {
        adicionarFilho($1, $2);
        $$ = $1;
    }
    ;

case_switch:
    TOKEN_CASE expressao TOKEN_DOIS_PONTOS lista_comandos
    {
        $$ = criarNo(NO_CASE, 0);
        adicionarFilho($$, $2);
        adicionarFilho($$, $4);
    }
    ;

default_switch:
      %empty { $$ = NULL; }
    | TOKEN_DEFAULT TOKEN_DOIS_PONTOS lista_comandos
    {
        $$ = criarNo(NO_DEFAULT, 0);
        adicionarFilho($$, $3);
    }
    ;

condicional:
      comando_if { $$ = $1; }
    | comando_switch { $$ = $1; }
    ;

/* Laços de repetição */

atribuicao_for:
      TOKEN_PTOVIRG { $$ = NULL; }
    | ID TOKEN_ATRIB expressao TOKEN_PTOVIRG 
        { 
            $$ = criarNo(NO_ATRIBUICAO, 0); 
            NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
            id->valor.valor_str = $1;
            adicionarFilho($$, id);
            adicionarFilho($$, $3);
        }
    | declaracao_local { $$ = $1; }
    ;

condicao_for:
      TOKEN_PTOVIRG { $$ = NULL; }
    | condicao TOKEN_PTOVIRG { $$ = $1; }
    ;

incremento_for:
      %empty { $$ = NULL; }
    | ID TOKEN_ATRIB expressao 
    { 
        $$ = criarNo(NO_ATRIBUICAO, 0);
        NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
        id->valor.valor_str = $1;
        adicionarFilho($$, id);
        adicionarFilho($$, $3);
    }
    | ID TOKEN_INCREMENTO
    {
        $$ = criarNo(NO_INCREMENTO, 0);
        NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
        id->valor.valor_str = $1;
        adicionarFilho($$, id);
    }
    | ID TOKEN_DECREMENTO
    {
        $$ = criarNo(NO_DECREMENTO, 0);
        NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
        id->valor.valor_str = $1;
        adicionarFilho($$, id);
    }
    | TOKEN_INCREMENTO ID
    {
        $$ = criarNo(NO_INCREMENTO_ANTES, 0);
        NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
        id->valor.valor_str = $2;
        adicionarFilho($$, id);
    }
    | TOKEN_DECREMENTO ID
    {
        $$ = criarNo(NO_DECREMENTO_ANTES, 0);
        NoAST *id = criarNo(NO_IDENTIFICADOR, 0);
        id->valor.valor_str = $2;
        adicionarFilho($$, id);
    }
    ;

repeticao:
      comando_for { $$ = $1; }
    | comando_do_while { $$ = $1; }
    | comando_while { $$ = $1; }
    ;

comando_for:
    TOKEN_FOR TOKEN_ABRE_PAR atribuicao_for condicao_for incremento_for TOKEN_FECHA_PAR bloco
    {
        $$ = criarNo(NO_FOR, 0);
        adicionarFilho($$, $3);
        adicionarFilho($$, $4);
        adicionarFilho($$, $5);
        adicionarFilho($$, $7);
    }
    ;

comando_do_while:
    TOKEN_DO bloco TOKEN_WHILE TOKEN_ABRE_PAR condicao TOKEN_FECHA_PAR TOKEN_PTOVIRG
    {
        $$ = criarNo(NO_DO_WHILE, 0);
        adicionarFilho($$, $2);
        adicionarFilho($$, $5);
    }
    ;

comando_while:
    TOKEN_WHILE TOKEN_ABRE_PAR condicao TOKEN_FECHA_PAR bloco
    {
        $$ = criarNo(NO_WHILE, 0);
        adicionarFilho($$, $3);
        adicionarFilho($$, $5);
    }
    ;

%%

void yyerror(const char *mensagem) {
    fprintf(stderr, "Erro sintatico na linha %d: %s\n", yylineno, mensagem);
}

#ifndef ANALISE_SEMANTICA
int main(void) {
    if (yyparse() == 0) {
        if (arvore) {
            imprimirAST(arvore);
        }
        printf("\nAnalise sintatica concluida com sucesso.\n");
        liberarAST(arvore);
        return 0;
    }
    return 1;
}
#endif
/*

abstract de fora
*/
