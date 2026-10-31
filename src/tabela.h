#ifndef TABELA_H
#define TABELA_H

#include "tipos.h"

#define TAM_TABELA_HASH 31

/*
 * Estrutura para representar modificadores de variaveis e metodos:
 * public, private, protected, static e final.
 */
typedef struct {
    int eh_public;
    int eh_private;
    int eh_protected;
    int eh_static;
    int eh_final;
} Modificadores;

#define MODIFICADORES_PADRAO ((Modificadores){0, 0, 0, 0, 0})

/*
 * Estrutura do Simbolo na Tabela de Simbolos.
 */
typedef struct Simbolo {
    char *nome;
    TipoDado tipo;
    Modificadores mods;
    int linha;
    struct Simbolo *proximo; /* Lista encadeada para colisao na hash ou encadeamento */
} Simbolo;

/*
 * Estrutura do Escopo:
 * Possui tabela hash de simbolos e ponteiro para o escopo pai (enclosing scope),
 * permitindo uma pilha hierarquica de escopos.
 */
typedef struct Escopo {
    Simbolo *tabela[TAM_TABELA_HASH];
    struct Escopo *pai;
    int nivel;
    struct Escopo *proximo_alocado; /* Encadeamento de todos os escopos alocados */
} Escopo;

/*
 * Inicializa a infraestrutura da tabela de simbolos e pilha de escopos.
 */
void inicializarTabelaSimbolos(void);

/*
 * Cria um novo escopo aninhado e o define como o escopo corrente (topo da pilha).
 */
void empilharEscopo(void);

/*
 * Fecha e desaloca o escopo corrente, retornando o controle ao escopo pai.
 */
void desempilharEscopo(void);

/*
 * Insere um novo simbolo no escopo corrente.
 * Retorna 1 em caso de sucesso.
 * Retorna 0 caso o identificador ja exista no escopo corrente (redeclaração proibida).
 */
int inserirSimbolo(char *nome, TipoDado tipo, Modificadores mods, int linha);

/*
 * Realiza a busca ascendente por um identificador, comecando no escopo corrente
 * e subindo a cadeia de pais ate o escopo global.
 * Retorna o ponteiro para o Simbolo se encontrado, ou NULL caso contrario.
 */
Simbolo *buscarSimbolo(char *nome);

/*
 * Busca um identificador exclusivamente no escopo corrente.
 * Util para validacao de redeclaracao local.
 */
Simbolo *buscarSimboloEscopoAtual(char *nome);

/*
 * Retorna o escopo corrente (topo da pilha de escopos).
 */
Escopo *obterEscopoAtual(void);

/*
 * Desaloca recursivamente todos os escopos e simbolos da pilha.
 */
void liberarTabelaSimbolos(void);

#endif /* TABELA_H */
