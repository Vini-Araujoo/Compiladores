#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

#define CAPACIDADE_INICIAL_FILHOS 4

NoAST *criarNo(TipoNoAST tipo, int linha) {
    NoAST *no = (NoAST *)malloc(sizeof(NoAST));
    if (!no) {
        perror("Erro ao alocar memoria para NoAST");
        exit(EXIT_FAILURE);
    }

    no->tipo_no = tipo;
    no->tipo_dado = TIPO_VOID;
    no->linha = linha;
    no->posfixo = 0;
    no->num_filhos = 0;
    no->capacidade = CAPACIDADE_INICIAL_FILHOS;
    memset(&no->valor, 0, sizeof(ValorLiteral));

    no->filhos = (NoAST **)malloc(sizeof(NoAST *) * no->capacidade);
    if (!no->filhos) {
        perror("Erro ao alocar vetor de filhos");
        free(no);
        exit(EXIT_FAILURE);
    }

    return no;
}

void adicionarFilho(NoAST *pai, NoAST *filho) {
    if (!pai) {
        return;
    }

    if (pai->num_filhos >= pai->capacidade) {
        int nova_capacidade = (pai->capacidade == 0) ? CAPACIDADE_INICIAL_FILHOS : pai->capacidade * 2;
        NoAST **novos_filhos = (NoAST **)realloc(pai->filhos, sizeof(NoAST *) * nova_capacidade);
        if (!novos_filhos) {
            perror("Erro ao redimensionar vetor de filhos");
            exit(EXIT_FAILURE);
        }
        pai->filhos = novos_filhos;
        pai->capacidade = nova_capacidade;
    }

    pai->filhos[pai->num_filhos++] = filho;
}

static const char *nomeTipoNoAST(TipoNoAST tipo) {
    static const char *const nomes[] = {
        "NO_PROGRAMA", "NO_CLASSE", "NO_METODO", "NO_PARAMETROS", "NO_PARAMETRO",
        "NO_DECL_VARIAVEL", "NO_BLOCO", "NO_IF", "NO_ELSE", "NO_WHILE",
        "NO_FOR", "NO_DO_WHILE", "NO_SWITCH", "NO_CASES", "NO_CASE",
        "NO_DEFAULT", "NO_RETURN", "NO_BREAK", "NO_INCREMENTO",
        "NO_DECREMENTO", "NO_INCREMENTO_ANTES", "NO_DECREMENTO_ANTES",
        "NO_ATRIBUICAO", "NO_PRINT", "NO_PRINTLN", "NO_INSTANCIACAO_SCANNER",
        "NO_LEITURA_SCANNER",
        "NO_OP_BINARIA", "NO_OP_UNARIA", "NO_CHAMADA_METODO",
        "NO_NOVA_INSTANCIA", "NO_INSTANCIACAO_ARRAY", "NO_ACESSO_ARRAY", "NO_CONDICAO",
        "NO_EXPRESSAO", "NO_THEN", "NO_IDENTIFICADOR", "NO_LITERAL_INT",
        "NO_LITERAL_FLOAT", "NO_LITERAL_DOUBLE", "NO_LITERAL_CHAR",
        "NO_LITERAL_STRING", "NO_LITERAL_BOOL"
    };
    const size_t quantidade = sizeof(nomes) / sizeof(nomes[0]);

    if ((unsigned int)tipo >= quantidade) {
        return "NO_DESCONHECIDO";
    }
    return nomes[tipo];
}

static const char *rotuloFilho(const NoAST *pai, int indice) {
    if (pai->tipo_no == NO_FOR) {
        static const char *const segmentos[] = {
            "inicializacao", "condicao", "incremento", "bloco"
        };
        if (indice < 4) {
            return segmentos[indice];
        }
    }
    if (pai->tipo_no == NO_SWITCH) {
        static const char *const segmentos[] = {"seletor", "casos", "default"};
        if (indice < 3) {
            return segmentos[indice];
        }
    }
    if (pai->tipo_no == NO_CASES) {
        return "case";
    }
    if (pai->tipo_no == NO_CASE) {
        return indice == 0 ? "valor" : "comandos";
    }
    if (pai->tipo_no == NO_DEFAULT) {
        return "comandos";
    }
    if (pai->tipo_no == NO_INSTANCIACAO_ARRAY)
    {
        return "tamanho";
    }
    if (pai->tipo_no == NO_ACESSO_ARRAY)
    {
        return indice == 0 ? "array" : "indice";
    }
    if (pai->tipo_no == NO_PRINT || pai->tipo_no == NO_PRINTLN)
    {
        return "argumento";
    }
    if (pai->tipo_no == NO_LEITURA_SCANNER)
    {
        return "variavel";
    }
    
    
    return NULL;
}

static void imprimirASTRec(const NoAST *no, const char *prefixo,
                          int ultimo, const char *rotulo) {
    if (rotulo) {
        printf("%s%s%s: ", prefixo, ultimo ? "└── " : "├── ", rotulo);
    } else if (*prefixo) {
        printf("%s%s", prefixo, ultimo ? "└── " : "├── ");
    }

    if (!no) {
        puts("<vazio>");
        return;
    }

    printf("%s", nomeTipoNoAST(no->tipo_no));
    if ((no->tipo_no == NO_CLASSE || no->tipo_no == NO_IDENTIFICADOR ||
         no->tipo_no == NO_METODO || no->tipo_no == NO_PARAMETRO ||
         no->tipo_no == NO_DECL_VARIAVEL ||
         no->tipo_no == NO_LEITURA_SCANNER) &&
        no->valor.valor_str) {
        printf(" (%s)", no->valor.valor_str);
    } else if (no->tipo_no == NO_ATRIBUICAO && no->valor.operador[0] &&
               strcmp(no->valor.operador, "=") != 0) {
        printf(" (%s)", no->valor.operador);
    } else if (no->tipo_no == NO_LITERAL_INT) {
        printf(" (%ld)", no->valor.valor_int);
    } else if (no->tipo_no == NO_LITERAL_FLOAT || no->tipo_no == NO_LITERAL_DOUBLE) {
        printf(" (%g)", no->valor.valor_double);
    } else if (no->tipo_no == NO_LITERAL_STRING && no->valor.valor_str) {
        printf(" (\"%s\")", no->valor.valor_str);
    } else if (no->tipo_no == NO_LITERAL_CHAR) {
        printf(" ('%c')", (char)no->valor.valor_int);
    } else if (no->tipo_no == NO_LITERAL_BOOL) {
        printf(" (%s)", no->valor.valor_int ? "true" : "false");
    } else if ((no->tipo_no == NO_OP_BINARIA || no->tipo_no == NO_OP_UNARIA) &&
               no->valor.operador[0]) {
        printf(" (%s%s)", no->valor.operador,
               (no->tipo_no == NO_OP_UNARIA && no->posfixo) ? ", posfixo" : "");
    }
    putchar('\n');

    char proximo_prefixo[512];
    size_t tamanho_prefixo = strlen(prefixo);
    if (tamanho_prefixo > sizeof(proximo_prefixo) - 5) {
        tamanho_prefixo = sizeof(proximo_prefixo) - 5;
    }
    memcpy(proximo_prefixo, prefixo, tamanho_prefixo);
    proximo_prefixo[tamanho_prefixo] = '\0';
    if (rotulo || *prefixo) {
        strncat(proximo_prefixo, ultimo ? "    " : "│   ",
                sizeof(proximo_prefixo) - strlen(proximo_prefixo) - 1);
    }

    for (int i = 0; i < no->num_filhos; i++) {
        int filho_final = i == no->num_filhos - 1;
        char rotulo_generico[32];
        const char *rotulo_filho = rotuloFilho(no, i);
        if (!rotulo_filho) {
            snprintf(rotulo_generico, sizeof(rotulo_generico), "filho[%d]", i);
            rotulo_filho = rotulo_generico;
        }
        imprimirASTRec(no->filhos[i], proximo_prefixo, filho_final,
                       rotulo_filho);
    }
}

void imprimirAST(const NoAST *raiz) {
    imprimirASTRec(raiz, "", 1, NULL);
}

void liberarAST(NoAST *raiz) {
    if (!raiz) {
        return;
    }

    for (int i = 0; i < raiz->num_filhos; i++) {
        liberarAST(raiz->filhos[i]);
    }

    if (raiz->filhos) {
        free(raiz->filhos);
        raiz->filhos = NULL;
    }

    if (raiz->tipo_no == NO_CLASSE ||
        raiz->tipo_no == NO_IDENTIFICADOR ||
        raiz->tipo_no == NO_METODO ||
        raiz->tipo_no == NO_PARAMETRO ||
        raiz->tipo_no == NO_DECL_VARIAVEL ||
        raiz->tipo_no == NO_LEITURA_SCANNER ||
        raiz->tipo_no == NO_LITERAL_STRING) {
        free(raiz->valor.valor_str);
    }
    free(raiz);
}
