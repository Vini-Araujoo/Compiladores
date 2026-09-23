%{
#include <stdio.h>

int yylex(void);
void yyerror(const char *mensagem);
%}

// primitivos
%token TOKEN_BYTE
%token TOKEN_SHORT
%token ID
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

%start programa

%nonassoc LOWER_THAN_ELSE
%nonassoc TOKEN_ELSE

%%

/* Estrutura do programa e blocos */

programa:
      declaracao_classe
    | %empty
    ;
declaracao_classe:
        TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
      | TOKEN_FINAL TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
      | TOKEN_PUBLIC TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE 
      | TOKEN_PUBLIC TOKEN_FINAL TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
    ;

corpo_classe:
      %empty
      | corpo_classe membro_classe
    ;

membro_classe:
        declaracao_variavel
      | declaracao_metodo  
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
    TOKEN_ABRE_CHAVE lista_comandos TOKEN_FECHA_CHAVE
    ;

lista_comandos:
      %empty
    | lista_comandos comando
    ;

comando:
  declaracao_local
    | atribuicao
    | condicional
    | repeticao
    | comando_salto
    | bloco
    ;

/* Expressões e comandos básicos */

expressao:
      ID
    | NUMERO
    ;

declaracao_local:
      modificador_variavel tipo lista_declaracoes TOKEN_PTOVIRG
    ;

atribuicao:
      ID TOKEN_ATRIB expressao TOKEN_PTOVIRG
    | TOKEN_INCREMENTO ID TOKEN_PTOVIRG
    | TOKEN_DECREMENTO ID TOKEN_PTOVIRG
    | ID TOKEN_INCREMENTO TOKEN_PTOVIRG
    | ID TOKEN_DECREMENTO TOKEN_PTOVIRG
    | ID TOKEN_SOMA_ATRIB expressao TOKEN_PTOVIRG
    | ID TOKEN_SUB_ATRIB expressao TOKEN_PTOVIRG
    ;

comando_return:
      TOKEN_RETURN TOKEN_PTOVIRG
    | TOKEN_RETURN expressao TOKEN_PTOVIRG
    ;

comando_salto:
      TOKEN_BREAK TOKEN_PTOVIRG
    | comando_return
    ;

/* Condições: parênteses/comparações > && > || */

comparador:
      TOKEN_IGUAL
    | TOKEN_DIFERENTE
    | TOKEN_MENOR
    | TOKEN_MAIOR
    | TOKEN_MENOR_IGUAL
    | TOKEN_MAIOR_IGUAL
    ;

condicao:
    condicao_ou
    ;

condicao_ou:
      condicao_ou TOKEN_OU condicao_e
    | condicao_e
    ;

condicao_e:
      condicao_e TOKEN_E condicao_base
    | condicao_base
    ;

condicao_base:
      TOKEN_ABRE_PAR condicao TOKEN_FECHA_PAR
    | TOKEN_TRUE
    | TOKEN_FALSE
    | expressao comparador expressao
    | expressao
    ;

/* Condicionais */

comando_if:
      TOKEN_IF TOKEN_ABRE_PAR condicao TOKEN_FECHA_PAR comando
        %prec LOWER_THAN_ELSE
    | TOKEN_IF TOKEN_ABRE_PAR condicao TOKEN_FECHA_PAR comando
      TOKEN_ELSE comando
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
      comando_if
    | comando_switch
    ;

/* Laços de repetição */

atribuicao_for:
      TOKEN_PTOVIRG
    | ID TOKEN_ATRIB expressao TOKEN_PTOVIRG
    | declaracao_local
    ;

condicao_for:
      TOKEN_PTOVIRG
    | condicao TOKEN_PTOVIRG
    ;

incremento_for:
      %empty
    | ID TOKEN_ATRIB expressao
    | ID TOKEN_INCREMENTO
    | ID TOKEN_DECREMENTO
    | TOKEN_INCREMENTO ID
    | TOKEN_DECREMENTO ID
    ;

repeticao:
      comando_for
    | comando_do_while
    | comando_while
    ;

comando_for:
    TOKEN_FOR TOKEN_ABRE_PAR atribuicao_for condicao_for incremento_for
    TOKEN_FECHA_PAR bloco
    ;

comando_do_while:
    TOKEN_DO bloco TOKEN_WHILE TOKEN_ABRE_PAR condicao TOKEN_FECHA_PAR
    TOKEN_PTOVIRG
    ;

comando_while:
    TOKEN_WHILE TOKEN_ABRE_PAR condicao TOKEN_FECHA_PAR bloco
    ;

%%

void yyerror(const char *mensagem) {
    fprintf(stderr, "Erro sintatico: %s\n", mensagem);
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
