
%{
#include <stdio.h>

/*por enquanto utilizando um enum mas substituir pelo arquivo do bison*/
#include "enum/tokens.h"

//#define ID 3
//#define NUMERO 4
%}


%%
"//".*              { /* ignora linha de comentário */ }
"/*"([^*]|\*+[^*/])*\*+"/" { /* ignora bloco de comentário */ }

"if"            { return TOKEN_IF;              }
"else"          { return TOKEN_ELSE;            }
"while"         { return TOKEN_WHILE;           }
"for"           { return TOKEN_FOR;             }
"return"        { return TOKEN_RETURN;          }
"byte"          { return TOKEN_BYTE;            }
"short"         { return TOKEN_SHORT;           }
"int"           { return TOKEN_INT;             }
"long"          { return TOKEN_LONG;            }
"boolean"       { return TOKEN_BOOLEAN;         }
"char"          { return TOKEN_CHAR;            }
"float"         { return TOKEN_FLOAT;           }
"double"        { return TOKEN_DOUBLE;          }

"switch"        { return TOKEN_SWITCH;           }
"case"          { return TOKEN_CASE;             }
"default"       { return TOKEN_DEFALUT;          }
"break"         { return TOKEN_BREAK;            }
"do"            { return TOKEN_DO;               }

"String"        { return TOKEN_STRING_TIPO;      }
"System"        { return TOKEN_SYSTEM;           }
"out"           { return TOKEN_OUT;              }
"in"            { return TOKEN_IN;               }
"println"       { return TOKEN_PRINTLN;          }
"print"         { return TOKEN_PRINT;            }
"Scanner"       { return TOKEN_SCANNER;          }
"nextLine"      { return TOKEN_NEXT_LINE;       }
"nextInt"       { return TOKEN_NEXT_INT;        }
"nextDouble"    { return TOKEN_NEXT_DOUBLE;     }
"nextFloat"     { return TOKEN_NEXT_FLOAT;      }
"nextLong"      { return TOKEN_NEXT_LONG;       }
"nextShort"     { return TOKEN_NEXT_SHORT;      }
"nextByte"      { return TOKEN_NEXT_BYTE;       }
"nextBoolean"   { return TOKEN_NEXT_BOOLEAN;    }
"next"          { return TOKEN_NEXT;            }
"new"           { return TOKEN_NEW;             }

"public"        { return TOKEN_PUBLIC;           }
"private"       { return TOKEN_PRIVATE;          }
"protected"     { return TOKEN_PROTECTED;        }
"static"        { return TOKEN_STATIC;           }
"class"         { return TOKEN_CLASS;            }
"void"          { return TOKEN_VOID;             }
"final"         { return TOKEN_FINAL;            }

    /* igualdade tem precedencia em relação ao de atribuição */
"=="            { return TOKEN_IGUAL;           }
"!="            { return TOKEN_DIFERENTE;       }
"<="            { return TOKEN_MENOR_IGUAL;     }
">="            { return TOKEN_MAIOR_IGUAL;     }
"+="            { return TOKEN_SOMA_ATRIB;      }
"-="            { return TOKEN_SUB_ATRIB;       }
"++"            { return TOKEN_INCREMENTO;      }
"--"            { return TOKEN_DECREMENTO;      }
"<"             { return TOKEN_MENOR;           }
">"             { return TOKEN_MAIOR;           }
"="             { return TOKEN_ATRIB;           }
"!"             { return TOKEN_NEGACAO;         }
"+"             { return TOKEN_MAIS;            }
"-"             { return TOKEN_MENOS;           }
"*"             { return TOKEN_VEZES;           }
"/"             { return TOKEN_DIV;             } 
";"             { return TOKEN_PTOVIRG;         }
","             { return TOKEN_VIRGULA;         }
":"             { return TOKEN_DOIS_PONTOS;      }
"("             { return TOKEN_ABRE_PAR;        }
")"             { return TOKEN_FECHA_PAR;       }
"{"             { return TOKEN_ABRE_CHAVE;      }
"}"             { return TOKEN_FECHA_CHAVE;     }
"["             { return TOKEN_ABRE_COLCHETE;   }
"]"             { return TOKEN_FECHA_COLCHETE;  }
"."             { return TOKEN_PONTO;           }


[a-zA-Z_][a-zA-Z_0-9]*  { return ID; }
[0-9]+                  { return NUMERO; }
[ \t\n]+ {/* ignora espaços e quebra de linha */}

. {printf("Erro: caractere desconhecido");/* ignora qualquer outro caractere */}

%%

int yywrap(void){
    return 1;
}


int main(int argc, char **argv){
    //return yyparse(); chama a função principal do bison (tenta validar a gramatica das frases )

    int token;

    while((token = yylex()) != 0){ //yylex consome entrada ate encontrar num valido, em caso de EOF entao yylex = 0
        printf("lexema: %s | token: %d\n", yytext, token);
        
    }

    return 0;
}