%{
#include <stdio.h>
#include <stdlib.h>
#include "../src/ast.h"

int yylex(void);
void yyerror(const char *mensagem);
extern int yylineno;

NoAST *arvore = NULL;
%}

// primitivos
%token TOKEN_BYTE
%token TOKEN_SHORT
%token <text> ID
%token NUMERO
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
%left   TOKEN_OU
%left   TOKEN_E
%left   TOKEN_IGUAL TOKEN_DIFERENTE
%left   TOKEN_MENOR TOKEN_MAIOR TOKEN_MENOR_IGUAL TOKEN_MAIOR_IGUAL
%left   TOKEN_MAIS TOKEN_MENOS
%left   TOKEN_VEZES TOKEN_DIV
%precedence TOKEN_NEGACAO UMINUS

%start programa

%precedence LOWER_THAN_ELSE
%precedence TOKEN_ELSE

%code requires {
    #include "../src/ast.h"
}

%union {
    NoAST *no;
    char *text;
}

%destructor { free($$); } <text>

%type <no> programa declaracao_classe corpo_classe membro_classe
%type <no> bloco comando lista_comandos condicional condicao comando_if
%type <no> declaracao_local atribuicao comando_io repeticao comando_salto
%type <no> expressao
%type <no> atribuicao_for condicao_for incremento_for
%type <no> comando_for comando_do_while comando_while

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
        { $$ = $4; }
      | TOKEN_FINAL TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
        { $$ = $5; }
      | TOKEN_PUBLIC TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
        { $$ = $5; }
      | TOKEN_PUBLIC TOKEN_FINAL TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
        { $$ = $6; }
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
      | declaracao_metodo { $$ = criarNo(NO_METODO, 0); }
    ;


declaracao_metodo:
  modificadores tipo ID TOKEN_ABRE_PAR lista_parametros TOKEN_FECHA_PAR bloco
    | modificadores TOKEN_VOID ID TOKEN_ABRE_PAR lista_parametros TOKEN_FECHA_PAR bloco
    ;

lista_parametros:
        %empty
      | parametros_com_virgula
    ;

parametros_com_virgula:
        parametro
      | parametros_com_virgula TOKEN_VIRGULA parametro
    ;

parametro:
        tipo ID
      | tipo TOKEN_ABRE_COLCHETE TOKEN_FECHA_COLCHETE ID /*para String[] args*/  
    
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
      | TOKEN_INT
      | TOKEN_STRING_TIPO
      | TOKEN_LONG
      | TOKEN_BOOLEAN
      | TOKEN_SHORT
      | TOKEN_CHAR
      | TOKEN_FLOAT
      | TOKEN_SCANNER
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
      expressao_sintatica { $$ = criarNo(NO_EXPRESSAO, 0); }
    ;

/* Stub: valida a sintaxe da expressão sem construir sua subárvore. */
expressao_sintatica:
      expressao_sintatica TOKEN_OU expressao_sintatica
    | expressao_sintatica TOKEN_E expressao_sintatica
    | expressao_sintatica TOKEN_MAIS expressao_sintatica
    | expressao_sintatica TOKEN_MENOS expressao_sintatica
    | expressao_sintatica TOKEN_VEZES expressao_sintatica
    | expressao_sintatica TOKEN_DIV expressao_sintatica
    | expressao_sintatica TOKEN_MENOR expressao_sintatica
    | expressao_sintatica TOKEN_MAIOR expressao_sintatica
    | expressao_sintatica TOKEN_MENOR_IGUAL expressao_sintatica
    | expressao_sintatica TOKEN_MAIOR_IGUAL expressao_sintatica
    | expressao_sintatica TOKEN_IGUAL expressao_sintatica
    | expressao_sintatica TOKEN_DIFERENTE expressao_sintatica
    | TOKEN_NEGACAO expressao_sintatica
    | TOKEN_MENOS expressao_sintatica %prec UMINUS
    | TOKEN_ABRE_PAR expressao_sintatica TOKEN_FECHA_PAR
    | ID { free($1); }
    | NUMERO
    | TOKEN_TRUE
    | TOKEN_FALSE
    | acesso_array
    | leitura_scanner
    | chamada_metodo
    | instanciacao
    ;

declaracao_local:
      modificador_variavel tipo lista_declaracoes TOKEN_PTOVIRG
    ;

atribuicao:
      ID TOKEN_ATRIB expressao TOKEN_PTOVIRG
  | acesso_array TOKEN_ATRIB expressao TOKEN_PTOVIRG
    | TOKEN_INCREMENTO ID TOKEN_PTOVIRG
    | TOKEN_DECREMENTO ID TOKEN_PTOVIRG
    | ID TOKEN_INCREMENTO TOKEN_PTOVIRG
    | ID TOKEN_DECREMENTO TOKEN_PTOVIRG
    | ID TOKEN_SOMA_ATRIB expressao TOKEN_PTOVIRG
    | ID TOKEN_SUB_ATRIB expressao TOKEN_PTOVIRG
    ;

/* Entrada, saída, instanciação, arrays e chamadas qualificadas */

comando_io:
      comando_print
    | expressao TOKEN_PTOVIRG
    ;

lista_instrucoes_io:
      comando_io
    | atribuicao
    | lista_instrucoes_io comando_io
    | lista_instrucoes_io atribuicao
    ;

comando_print:
      TOKEN_SYSTEM TOKEN_PONTO TOKEN_OUT TOKEN_PONTO TOKEN_PRINTLN
      TOKEN_ABRE_PAR argumento_opcional TOKEN_FECHA_PAR TOKEN_PTOVIRG
    | TOKEN_SYSTEM TOKEN_PONTO TOKEN_OUT TOKEN_PONTO TOKEN_PRINT
      TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR TOKEN_PTOVIRG
    ;

argumento_opcional:
      %empty
    | expressao
    ;

instanciacao:
      TOKEN_NEW TOKEN_SCANNER TOKEN_ABRE_PAR TOKEN_SYSTEM TOKEN_PONTO
      TOKEN_IN TOKEN_FECHA_PAR
    | TOKEN_NEW tipo_base TOKEN_ABRE_COLCHETE expressao TOKEN_FECHA_COLCHETE
    ;

tipo_base:
      TOKEN_INT
    | TOKEN_DOUBLE
    | TOKEN_STRING_TIPO
    ;

acesso_array:
      ID TOKEN_ABRE_COLCHETE expressao TOKEN_FECHA_COLCHETE
    ;

leitura_scanner:
      ID TOKEN_PONTO metodo_scanner TOKEN_ABRE_PAR TOKEN_FECHA_PAR
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
    | ID TOKEN_PONTO ID TOKEN_ABRE_PAR argumentos_opcionais TOKEN_FECHA_PAR
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
      TOKEN_RETURN TOKEN_PTOVIRG
    | TOKEN_RETURN expressao TOKEN_PTOVIRG
    ;

comando_salto:
      TOKEN_BREAK TOKEN_PTOVIRG
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
    ;

casos_switch:
      %empty
    | casos_switch case_switch
    ;

case_switch:
    TOKEN_CASE expressao TOKEN_DOIS_PONTOS lista_comandos
    ;

default_switch:
      %empty
    | TOKEN_DEFAULT TOKEN_DOIS_PONTOS lista_comandos
    ;

condicional:
      comando_if { $$ = $1; }
    | comando_switch { $$ = criarNo(NO_SWITCH, 0); }
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

int main(void) {
    if (yyparse() == 0) {
        printf("\nAnalise sintatica concluida com sucesso.\n");
        return 0;
    }
    return 1;
}
/*

abstract de fora
*/
