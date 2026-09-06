#ifndef TOKENS
#define TOKENS

enum
{
    // bsil bcfd

    // primitivos
    TOKEN_BYTE = 1,
    TOKEN_SHORT,
    ID,
    NUMERO,
    TOKEN_INT,
    TOKEN_LONG,
    TOKEN_BOOLEAN,
    TOKEN_CHAR,
    TOKEN_FLOAT,
    TOKEN_DOUBLE,

    // operadores
    TOKEN_ATRIB,
    TOKEN_IGUAL,
    TOKEN_DIFERENTE,
    TOKEN_MENOR_IGUAL,
    TOKEN_MAIOR_IGUAL,
    TOKEN_MENOR,
    TOKEN_MAIOR,
    TOKEN_MAIS,
    TOKEN_MENOS,
    TOKEN_VEZES,
    TOKEN_DIV,

    // delimitadores
    TOKEN_PTOVIRG, //;
    TOKEN_VIRGULA,
    TOKEN_ABRE_PAR,  // (
    TOKEN_FECHA_PAR, // )
    TOKEN_ABRE_CHAVE,
    TOKEN_FECHA_CHAVE,

    // condicionais e repetição
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_DEFALUT,
    TOKEN_BREAK,
    TOKEN_DO,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,

    // identificação de alguns tokens de entrada em java
    TOKEN_STRING_TIPO,
    TOKEN_SYSTEM, // System
    TOKEN_OUT,    // out
    TOKEN_PRINTLN,
    TOKEN_PRINT,
    TOKEN_SCANNER, // Scanner

    // modificadores e declaração de classe
    TOKEN_PUBLIC,
    TOKEN_PRIVATE,
    TOKEN_PROTECTED,
    TOKEN_STATIC,
    TOKEN_CLASS,
    TOKEN_VOID,  // retorno vazio
    TOKEN_FINAL, // const em java

    // identificação de tokens para arrays e acesso
    TOKEN_ABRE_COLCHETE,
    TOKEN_FECHA_COLCHETE,
    TOKEN_PONTO,

    // operadores adicionais
    TOKEN_SOMA_ATRIB,
    TOKEN_SUB_ATRIB,
    TOKEN_INCREMENTO,
    TOKEN_DECREMENTO,
    TOKEN_NEGACAO,

    // instancia e entrada
    TOKEN_NEW,
    TOKEN_NEXT,
    TOKEN_NEXT_LINE,
    TOKEN_NEXT_INT,
    TOKEN_NEXT_DOUBLE,
    TOKEN_NEXT_FLOAT,
    TOKEN_NEXT_LONG,
    TOKEN_NEXT_SHORT,
    TOKEN_NEXT_BYTE,
    TOKEN_NEXT_BOOLEAN,
    TOKEN_IN,
    TOKEN_DOIS_PONTOS,
};

#endif