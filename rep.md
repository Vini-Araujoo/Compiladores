# Gabarito de Referência do Parser (Bison/Yacc) — `rep.md`

Este documento contém a especificação técnica e as regras gramaticais de referência (gabarito) de cada uma das 5 partes do compilador. Ele serve como guia para o líder do projeto/avaliação e base para a integração final dos Pull Requests na branch `main`.

---

## Índice
1. [Gabarito da Issue 1: Tipos, Declarações e Atribuições](#gabarito-issue-1)
2. [Gabarito da Issue 2: Expressões e Precedência de Operadores](#gabarito-issue-2)
3. [Gabarito da Issue 3: Estruturas de Controle de Fluxo e Blocos](#gabarito-issue-3)
4. [Gabarito da Issue 4: Estrutura Global do Programa, Classes e Métodos](#gabarito-issue-4)
5. [Gabarito da Issue 5: Entrada/Saída (I/O), Instanciação e Arrays](#gabarito-issue-5)
6. [Arquivo Integrado Final: `parser/parser.y`](#arquivo-integrado-final)

---

<a name="gabarito-issue-1"></a>
## 1. Gabarito da Issue 1: Tipos, Declarações e Atribuições

### Escopo
Reconhecimento de tipos primitivos, `String`, modificador de constante `final`, declaração de variáveis simples e compostas (separadas por vírgula), e atribuições simples (`=`), compostas (`+=`, `-=`) e unárias (`++`, `--`).

### Regras Bison / BNF de Referência
```bison
tipo:
      tipo_primitivo
    | TOKEN_STRING_TIPO
    ;

tipo_primitivo:
      TOKEN_BYTE
    | TOKEN_SHORT
    | TOKEN_INT
    | TOKEN_LONG
    | TOKEN_BOOLEAN
    | TOKEN_CHAR
    | TOKEN_FLOAT
    | TOKEN_DOUBLE
    ;

modificador_variavel:
      TOKEN_FINAL
    ;

declaracao_variavel:
      tipo lista_declaradores TOKEN_PTOVIRG
    | modificador_variavel tipo lista_declaradores TOKEN_PTOVIRG
    ;

lista_declaradores:
      declarador
    | lista_declaradores TOKEN_VIRGULA declarador
    ;

declarador:
      ID
    | ID TOKEN_ATRIB expressao
    ;

atribuicao:
      ID TOKEN_ATRIB expressao
    | ID TOKEN_SOMA_ATRIB expressao
    | ID TOKEN_SUB_ATRIB expressao
    | ID TOKEN_INCREMENTO
    | ID TOKEN_DECREMENTO
    | TOKEN_INCREMENTO ID
    | TOKEN_DECREMENTO ID
    ;
```

---

<a name="gabarito-issue-2"></a>
## 2. Gabarito da Issue 2: Expressões e Precedência de Operadores

### Escopo
Expressões binárias aritméticas, relacionais, de igualdade, operadores unários (`!`, `-`), agrupamento de parênteses e resolução formal de precedência de operadores sem ambiguidades.

### Diretivas de Precedência e Regras Bison de Referência
```bison
// Declaração de precedência e associatividade (da menor para a maior prioridade)
%left TOKEN_IGUAL TOKEN_DIFERENTE
%left TOKEN_MENOR TOKEN_MAIOR TOKEN_MENOR_IGUAL TOKEN_MAIOR_IGUAL
%left TOKEN_MAIS TOKEN_MENOS
%left TOKEN_VEZES TOKEN_DIV
%right TOKEN_NEGACAO UMINUS

%%

expressao:
      expressao TOKEN_MAIS expressao
    | expressao TOKEN_MENOS expressao
    | expressao TOKEN_VEZES expressao
    | expressao TOKEN_DIV expressao
    | expressao TOKEN_IGUAL expressao
    | expressao TOKEN_DIFERENTE expressao
    | expressao TOKEN_MENOR expressao
    | expressao TOKEN_MAIOR expressao
    | expressao TOKEN_MENOR_IGUAL expressao
    | expressao TOKEN_MAIOR_IGUAL expressao
    | TOKEN_NEGACAO expressao
    | TOKEN_MENOS expressao %prec UMINUS
    | TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR
    | ID
    | NUMERO
    ;
```

---

<a name="gabarito-issue-3"></a>
## 3. Gabarito da Issue 3: Estruturas de Controle de Fluxo e Blocos

### Escopo
Condicionais `if` e `if-else` (com resolução formal do *dangling else*), seleção `switch-case-default`, repetições `while`, `do-while`, `for`, saltos `break`, `return` e blocos `{ ... }`.

### Diretivas de Resolução de Dangling Else e Regras Bison de Referência
```bison
// Precedência para associar o 'else' ao 'if' mais interno:
%nonassoc LOWER_THAN_ELSE
%nonassoc TOKEN_ELSE

%%

bloco:
      TOKEN_ABRE_CHAVE lista_comandos TOKEN_FECHA_CHAVE
    ;

lista_comandos:
      /* vazio */
    | lista_comandos comando
    ;

comando:
      bloco
    | declaracao_variavel
    | atribuicao TOKEN_PTOVIRG
    | comando_if
    | comando_switch
    | comando_while
    | comando_do_while
    | comando_for
    | comando_salto
    | TOKEN_PTOVIRG
    ;

comando_if:
      TOKEN_IF TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR comando %prec LOWER_THAN_ELSE
    | TOKEN_IF TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR comando TOKEN_ELSE comando
    ;

comando_switch:
      TOKEN_SWITCH TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR TOKEN_ABRE_CHAVE lista_cases TOKEN_FECHA_CHAVE
    ;

lista_cases:
      /* vazio */
    | lista_cases item_case
    ;

item_case:
      TOKEN_CASE NUMERO TOKEN_DOIS_PONTOS lista_comandos
    | TOKEN_DEFALUT TOKEN_DOIS_PONTOS lista_comandos
    ;

comando_while:
      TOKEN_WHILE TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR comando
    ;

comando_do_while:
      TOKEN_DO comando TOKEN_WHILE TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR TOKEN_PTOVIRG
    ;

comando_for:
      TOKEN_FOR TOKEN_ABRE_PAR init_for TOKEN_PTOVIRG cond_for TOKEN_PTOVIRG passo_for TOKEN_FECHA_PAR comando
    ;

init_for:
      /* vazio */
    | declaracao_variavel
    | atribuicao
    ;

cond_for:
      /* vazio */
    | expressao
    ;

passo_for:
      /* vazio */
    | atribuicao
    ;

comando_salto:
      TOKEN_BREAK TOKEN_PTOVIRG
    | TOKEN_RETURN TOKEN_PTOVIRG
    | TOKEN_RETURN expressao TOKEN_PTOVIRG
    ;
```

---

<a name="gabarito-issue-4"></a>
## 4. Gabarito da Issue 4: Estrutura Global do Programa, Classes e Métodos

### Escopo
Ponto de entrada `%start programa`, declaração de classes, modificadores de acesso/escopo (`public`, `private`, `protected`, `static`, `final`), membros de classe, declaração de métodos com assinaturas, parâmetros tipados (incluindo arrays como `String[] args`), retorno vazio `void` e corpo do método (`bloco`).

### Regras Bison de Referência
```bison
%start programa

%%

programa:
      lista_classes
    ;

lista_classes:
      declaracao_classe
    | lista_classes declaracao_classe
    ;

declaracao_classe:
      modificadores_opcionais TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
    ;

modificadores_opcionais:
      /* vazio */
    | modificadores
    ;

modificadores:
      modificador
    | modificadores modificador
    ;

modificador:
      TOKEN_PUBLIC
    | TOKEN_PRIVATE
    | TOKEN_PROTECTED
    | TOKEN_STATIC
    | TOKEN_FINAL
    ;

corpo_classe:
      /* vazio */
    | corpo_classe membro_classe
    ;

membro_classe:
      declaracao_metodo
    | modificadores_opcionais declaracao_variavel
    ;

tipo_retorno:
      tipo
    | TOKEN_VOID
    ;

declaracao_metodo:
      modificadores_opcionais tipo_retorno ID TOKEN_ABRE_PAR parametros_opcionais TOKEN_FECHA_PAR bloco
    ;

parametros_opcionais:
      /* vazio */
    | lista_parametros
    ;

lista_parametros:
      parametro
    | lista_parametros TOKEN_VIRGULA parametro
    ;

parametro:
      tipo ID
    | tipo TOKEN_ABRE_COLCHETE TOKEN_FECHA_COLCHETE ID
    ;
```

---

<a name="gabarito-issue-5"></a>
## 5. Gabarito da Issue 5: Entrada/Saída (I/O), Instanciação e Arrays

### Escopo
Comandos `System.out.println` e `System.out.print`, instanciação de `Scanner(System.in)`, métodos `next...`, instanciação com `new`, indexação de arrays (`ID[expressao]`) e chamadas de métodos.

### Regras Bison de Referência
```bison
comando_io:
      comando_print TOKEN_PTOVIRG
    | chamada_metodo TOKEN_PTOVIRG
    ;

comando_print:
      TOKEN_SYSTEM TOKEN_PONTO TOKEN_OUT TOKEN_PONTO TOKEN_PRINTLN TOKEN_ABRE_PAR argumento_print_opcional TOKEN_FECHA_PAR
    | TOKEN_SYSTEM TOKEN_PONTO TOKEN_OUT TOKEN_PONTO TOKEN_PRINT TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR
    ;

argumento_print_opcional:
      /* vazio */
    | expressao
    ;

instanciacao:
      TOKEN_NEW TOKEN_SCANNER TOKEN_ABRE_PAR TOKEN_SYSTEM TOKEN_PONTO TOKEN_IN TOKEN_FECHA_PAR
    | TOKEN_NEW tipo_base TOKEN_ABRE_COLCHETE expressao TOKEN_FECHA_COLCHETE
    ;

tipo_base:
      tipo_primitivo
    | TOKEN_STRING_TIPO
    ;

acesso_array:
      ID TOKEN_ABRE_COLCHETE expressao TOKEN_FECHA_COLCHETE
    ;

chamada_metodo:
      ID TOKEN_ABRE_PAR argumentos_opcionais TOKEN_FECHA_PAR
    | ID TOKEN_PONTO metodo_scanner TOKEN_ABRE_PAR TOKEN_FECHA_PAR
    | ID TOKEN_PONTO ID TOKEN_ABRE_PAR argumentos_opcionais TOKEN_FECHA_PAR
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

argumentos_opcionais:
      /* vazio */
    | lista_argumentos
    ;

lista_argumentos:
      expressao
    | lista_argumentos TOKEN_VIRGULA expressao
    ;
```

---

<a name="arquivo-integrado-final"></a>
## 6. Arquivo Integrado Final: `parser/parser.y`

Abaixo está o arquivo completo integrando todas as 5 partes consolidadas:

```bison
%{
#include <stdio.h>

int yylex(void);
void yyerror(const char *mensagem);
%}

// Primitivos e tipos
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
%token TOKEN_STRING_TIPO
%token TOKEN_VOID

// Operadores
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
%token TOKEN_SOMA_ATRIB
%token TOKEN_SUB_ATRIB
%token TOKEN_INCREMENTO
%token TOKEN_DECREMENTO
%token TOKEN_NEGACAO

// Delimitadores
%token TOKEN_PTOVIRG
%token TOKEN_VIRGULA
%token TOKEN_ABRE_PAR
%token TOKEN_FECHA_PAR
%token TOKEN_ABRE_CHAVE
%token TOKEN_FECHA_CHAVE
%token TOKEN_ABRE_COLCHETE
%token TOKEN_FECHA_COLCHETE
%token TOKEN_PONTO
%token TOKEN_DOIS_PONTOS

// Condicionais e repetição
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

// Modificadores e declaração de classe
%token TOKEN_PUBLIC
%token TOKEN_PRIVATE
%token TOKEN_PROTECTED
%token TOKEN_STATIC
%token TOKEN_CLASS
%token TOKEN_FINAL

// I/O e Scanner
%token TOKEN_SYSTEM
%token TOKEN_OUT
%token TOKEN_PRINTLN
%token TOKEN_PRINT
%token TOKEN_SCANNER
%token TOKEN_NEW
%token TOKEN_IN
%token TOKEN_NEXT
%token TOKEN_NEXT_LINE
%token TOKEN_NEXT_INT
%token TOKEN_NEXT_DOUBLE
%token TOKEN_NEXT_FLOAT
%token TOKEN_NEXT_LONG
%token TOKEN_NEXT_SHORT
%token TOKEN_NEXT_BYTE
%token TOKEN_NEXT_BOOLEAN

// Precedência de Operadores
%nonassoc LOWER_THAN_ELSE
%nonassoc TOKEN_ELSE

%left TOKEN_IGUAL TOKEN_DIFERENTE
%left TOKEN_MENOR TOKEN_MAIOR TOKEN_MENOR_IGUAL TOKEN_MAIOR_IGUAL
%left TOKEN_MAIS TOKEN_MENOS
%left TOKEN_VEZES TOKEN_DIV
%right TOKEN_NEGACAO UMINUS

%start programa

%%

/* =======================================================
   PARTE 4: Estrutura Global do Programa, Classes e Métodos
   ======================================================= */

programa:
      lista_classes
    ;

lista_classes:
      declaracao_classe
    | lista_classes declaracao_classe
    ;

declaracao_classe:
      modificadores_opcionais TOKEN_CLASS ID TOKEN_ABRE_CHAVE corpo_classe TOKEN_FECHA_CHAVE
    ;

modificadores_opcionais:
      /* vazio */
    | modificadores
    ;

modificadores:
      modificador
    | modificadores modificador
    ;

modificador:
      TOKEN_PUBLIC
    | TOKEN_PRIVATE
    | TOKEN_PROTECTED
    | TOKEN_STATIC
    | TOKEN_FINAL
    ;

corpo_classe:
      /* vazio */
    | corpo_classe membro_classe
    ;

membro_classe:
      declaracao_metodo
    | modificadores_opcionais declaracao_variavel
    ;

tipo_retorno:
      tipo
    | TOKEN_VOID
    ;

declaracao_metodo:
      modificadores_opcionais tipo_retorno ID TOKEN_ABRE_PAR parametros_opcionais TOKEN_FECHA_PAR bloco
    ;

parametros_opcionais:
      /* vazio */
    | lista_parametros
    ;

lista_parametros:
      parametro
    | lista_parametros TOKEN_VIRGULA parametro
    ;

parametro:
      tipo ID
    | tipo TOKEN_ABRE_COLCHETE TOKEN_FECHA_COLCHETE ID
    ;

/* =======================================================
   PARTE 1: Tipos, Declarações e Atribuições
   ======================================================= */

tipo:
      tipo_primitivo
    | TOKEN_STRING_TIPO
    ;

tipo_primitivo:
      TOKEN_BYTE
    | TOKEN_SHORT
    | TOKEN_INT
    | TOKEN_LONG
    | TOKEN_BOOLEAN
    | TOKEN_CHAR
    | TOKEN_FLOAT
    | TOKEN_DOUBLE
    ;

modificador_variavel:
      TOKEN_FINAL
    ;

declaracao_variavel:
      tipo lista_declaradores TOKEN_PTOVIRG
    | modificador_variavel tipo lista_declaradores TOKEN_PTOVIRG
    ;

lista_declaradores:
      declarador
    | lista_declaradores TOKEN_VIRGULA declarador
    ;

declarador:
      ID
    | ID TOKEN_ATRIB expressao
    | ID TOKEN_ATRIB instanciacao
    ;

atribuicao:
      ID TOKEN_ATRIB expressao
    | ID TOKEN_ATRIB instanciacao
    | acesso_array TOKEN_ATRIB expressao
    | ID TOKEN_SOMA_ATRIB expressao
    | ID TOKEN_SUB_ATRIB expressao
    | ID TOKEN_INCREMENTO
    | ID TOKEN_DECREMENTO
    | TOKEN_INCREMENTO ID
    | TOKEN_DECREMENTO ID
    ;

/* =======================================================
   PARTE 3: Comandos, Controle de Fluxo e Blocos
   ======================================================= */

bloco:
      TOKEN_ABRE_CHAVE lista_comandos TOKEN_FECHA_CHAVE
    ;

lista_comandos:
      /* vazio */
    | lista_comandos comando
    ;

comando:
      bloco
    | declaracao_variavel
    | atribuicao TOKEN_PTOVIRG
    | comando_if
    | comando_switch
    | comando_while
    | comando_do_while
    | comando_for
    | comando_salto
    | comando_io
    | TOKEN_PTOVIRG
    ;

comando_if:
      TOKEN_IF TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR comando %prec LOWER_THAN_ELSE
    | TOKEN_IF TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR comando TOKEN_ELSE comando
    ;

comando_switch:
      TOKEN_SWITCH TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR TOKEN_ABRE_CHAVE lista_cases TOKEN_FECHA_CHAVE
    ;

lista_cases:
      /* vazio */
    | lista_cases item_case
    ;

item_case:
      TOKEN_CASE NUMERO TOKEN_DOIS_PONTOS lista_comandos
    | TOKEN_DEFALUT TOKEN_DOIS_PONTOS lista_comandos
    ;

comando_while:
      TOKEN_WHILE TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR comando
    ;

comando_do_while:
      TOKEN_DO comando TOKEN_WHILE TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR TOKEN_PTOVIRG
    ;

comando_for:
      TOKEN_FOR TOKEN_ABRE_PAR init_for TOKEN_PTOVIRG cond_for TOKEN_PTOVIRG passo_for TOKEN_FECHA_PAR comando
    ;

init_for:
      /* vazio */
    | declaracao_variavel
    | atribuicao
    ;

cond_for:
      /* vazio */
    | expressao
    ;

passo_for:
      /* vazio */
    | atribuicao
    ;

comando_salto:
      TOKEN_BREAK TOKEN_PTOVIRG
    | TOKEN_RETURN TOKEN_PTOVIRG
    | TOKEN_RETURN expressao TOKEN_PTOVIRG
    ;

/* =======================================================
   PARTE 5: I/O, Instanciação e Arrays
   ======================================================= */

comando_io:
      comando_print TOKEN_PTOVIRG
    | chamada_metodo TOKEN_PTOVIRG
    ;

comando_print:
      TOKEN_SYSTEM TOKEN_PONTO TOKEN_OUT TOKEN_PONTO TOKEN_PRINTLN TOKEN_ABRE_PAR argumento_print_opcional TOKEN_FECHA_PAR
    | TOKEN_SYSTEM TOKEN_PONTO TOKEN_OUT TOKEN_PONTO TOKEN_PRINT TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR
    ;

argumento_print_opcional:
      /* vazio */
    | expressao
    ;

instanciacao:
      TOKEN_NEW TOKEN_SCANNER TOKEN_ABRE_PAR TOKEN_SYSTEM TOKEN_PONTO TOKEN_IN TOKEN_FECHA_PAR
    | TOKEN_NEW tipo TOKEN_ABRE_COLCHETE expressao TOKEN_FECHA_COLCHETE
    ;

acesso_array:
      ID TOKEN_ABRE_COLCHETE expressao TOKEN_FECHA_COLCHETE
    ;

chamada_metodo:
      ID TOKEN_ABRE_PAR argumentos_opcionais TOKEN_FECHA_PAR
    | ID TOKEN_PONTO metodo_scanner TOKEN_ABRE_PAR TOKEN_FECHA_PAR
    | ID TOKEN_PONTO ID TOKEN_ABRE_PAR argumentos_opcionais TOKEN_FECHA_PAR
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

argumentos_opcionais:
      /* vazio */
    | lista_argumentos
    ;

lista_argumentos:
      expressao
    | lista_argumentos TOKEN_VIRGULA expressao
    ;

/* =======================================================
   PARTE 2: Expressões e Precedência
   ======================================================= */

expressao:
      expressao TOKEN_MAIS expressao
    | expressao TOKEN_MENOS expressao
    | expressao TOKEN_VEZES expressao
    | expressao TOKEN_DIV expressao
    | expressao TOKEN_IGUAL expressao
    | expressao TOKEN_DIFERENTE expressao
    | expressao TOKEN_MENOR expressao
    | expressao TOKEN_MAIOR expressao
    | expressao TOKEN_MENOR_IGUAL expressao
    | expressao TOKEN_MAIOR_IGUAL expressao
    | TOKEN_NEGACAO expressao
    | TOKEN_MENOS expressao %prec UMINUS
    | TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR
    | acesso_array
    | chamada_metodo
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
```
