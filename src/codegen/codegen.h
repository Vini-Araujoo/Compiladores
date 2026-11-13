#ifndef CODEGEN
#define CODEGEN

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../ast/ast.h"
#include "../tipos.h"

// funcs de utilitarios (todos vao utilizar)
void emitirIndentacao(FILE *saida, int nivel);                     // o nivel serve pra identação e localização
void emitirLinha(FILE *saida, int nivel, const char *format, ...); // o ... serve pra passar varios argumentos
void emitirCabecalho(FILE *saida);

// parte de metodos classes e orq(kelyton: codegen_main.c)
void emitirMetodo(NoAST *no, FILE *saida, int nivel);
void emitirChamadaMetodo(NoAST *no, FILE *saida);
void emitirInstrucao(NoAST *no, FILE *saida, int nivel);
void gerarCodigoC(NoAST *raiz, FILE *saida);

// parte de controle de fluxo e loop (moretti: codegen_fluxo.c)
void emitirIf(NoAST *no, FILE *saida, int nivel);
void emitirWhile(NoAST *no, FILE *saida, int nivel);
void emitirDoWhile(NoAST *no, FILE *saida, int nivel);
void emitirFor(NoAST *no, FILE *saida, int nivel);
void emitirSwitch(NoAST *no, FILE *saida, int nivel);
void emitirBreak(NoAST *no, FILE *saida, int nivel);
void emitirReturn(NoAST *no, FILE *saida, int nivel);
void emitirComandoFluxo(NoAST *no, FILE *saida, int nivel);

// parte de expressao, literal e operador (pedro augusto: codegen_expr.c)
void emitirExpressao(NoAST *no, FILE *saida);
void emitirLiteral(NoAST *no, FILE *saida);
void emitirIdentificador(NoAST *no, FILE *saida);
void emitirOpBinaria(NoAST *no, FILE *saida);
void emitirOpUnaria(NoAST *no, FILE *saida);

// tipo, declaracao, atribuicao (vini: codegen_decl.c)
const char *traduzirTipoC(TipoDado tipo);
void emitirDeclaracaoVariavel(NoAST *no, FILE *saida, int nivel);
void emitirAtribuicao(NoAST *no, FILE *saida, int nivel);

// entrada, saida,array (natan: codegen_io.c)
void emitirPrintln(NoAST *no, FILE *saida, int nivel);
void emitirPrint(NoAST *no, FILE *saida, int nivel);
void emitirScannerLeitura(NoAST *no, FILE *saida, int nivel);
void emitirInstanciacaoArray(NoAST *no, FILE *saida, int nivel);
void emitirAcessoArray(NoAST *no, FILE *saida);

#endif // !1