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

    // precedencia e associatividade dos operadores de expressao,
    // da menor para a maior prioridade
%left   TOKEN_IGUAL TOKEN_DIFERENTE
%left   TOKEN_MENOR TOKEN_MAIOR TOKEN_MENOR_IGUAL TOKEN_MAIOR_IGUAL
%left   TOKEN_MAIS TOKEN_MENOS
%left   TOKEN_VEZES TOKEN_DIV
%precedence TOKEN_NEGACAO UMINUS

%start programa

%%

// Ponto de entrada temporario para validar expressoes isoladas (Issue 2).
// Aceita uma sequencia de expressoes terminadas em ';'.
programa:
      lista_expressoes
    ;

lista_expressoes:
      %empty
    | lista_expressoes expressao TOKEN_PTOVIRG
    ;

expressao:
      expressao TOKEN_MAIS expressao
    | expressao TOKEN_MENOS expressao
    | expressao TOKEN_VEZES expressao
    | expressao TOKEN_DIV expressao
    | expressao TOKEN_MENOR expressao
    | expressao TOKEN_MAIOR expressao
    | expressao TOKEN_MENOR_IGUAL expressao
    | expressao TOKEN_MAIOR_IGUAL expressao
    | expressao TOKEN_IGUAL expressao
    | expressao TOKEN_DIFERENTE expressao
    | TOKEN_NEGACAO expressao
    | TOKEN_MENOS expressao %prec UMINUS
    | TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR
    | ID
    | NUMERO
    ;

%%

void yyerror(const char *mensagem) {
    fprintf(stderr, "Erro sintatico: %s\n", mensagem);
}

int main(void) {
    if (yyparse() == 0) {
        printf("Analise sintatica concluida com sucesso.\n");
        return 0;
    }

    return 1;
}