%{
#include <stdio.h>

int yylex(void);
void yyerror(const char *mensagem);
%}

//%token TOKEN_INT
//%token ID
//%token NUMERO
//%token TOKEN_ATRIB
//%token TOKEN_PTOVIRG
 // bsil bcfd

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

    // delimitadores
%token TOKEN_PTOVIRG //;
%token TOKEN_VIRGULA
%token TOKEN_ABRE_PAR  // (
%token TOKEN_FECHA_PAR // )
%token TOKEN_ABRE_CHAVE
%token TOKEN_FECHA_CHAVE

    // condicionais e repetição
%token TOKEN_IF
%token TOKEN_ELSE
%token TOKEN_SWITCH
%token TOKEN_CASE
%token TOKEN_DEFALUT
%token TOKEN_BREAK
%token TOKEN_DO
%token TOKEN_WHILE
%token TOKEN_FOR
%token TOKEN_RETURN

    // identificação de alguns tokens de entrada em java
%token TOKEN_STRING_TIPO
%token TOKEN_SYSTEM // System
%token TOKEN_OUT // out
%token TOKEN_PRINTLN
%token TOKEN_PRINT
%token TOKEN_SCANNER // Scanner

    // modificadores e declaração de classe
%token TOKEN_PUBLIC
%token TOKEN_PRIVATE
%token TOKEN_PROTECTED
%token TOKEN_STATIC
%token TOKEN_CLASS
%token TOKEN_VOID  // retorno vazio
%token TOKEN_FINAL // const em java

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

%%

programa:
    declaracao_classe
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
        modificadores tipo ID TOKEN_PTOVIRG
      | modificadores tipo ID TOKEN_ATRIB NUMERO TOKEN_PTOVIRG
    ;

// (STUB da Issue 3 apenas abre e fecha chaves por enquanto)
bloco:
      TOKEN_ABRE_CHAVE TOKEN_FECHA_CHAVE
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