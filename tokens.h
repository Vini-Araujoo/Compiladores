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

    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
};

#endif