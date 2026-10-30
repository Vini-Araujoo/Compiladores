CC := gcc
FLEX := flex
BISON := bison

CFLAGS := -Wall -Wextra -Ibuild
BUILD_DIR := build

LEXER_SOURCE := lexer/lexer.l
LEXER_GENERATED := $(BUILD_DIR)/lex.yy.c

PARSER_SOURCE := parser/parser.y
PARSER_GENERATED_C := $(BUILD_DIR)/parser.tab.c
PARSER_GENERATED_H := $(BUILD_DIR)/parser.tab.h
PARSER_BINARY := $(BUILD_DIR)/parser

SEMANTIC_BINARY := $(BUILD_DIR)/semantic
.PHONY: all parser semantic test test-ast test-semantic clean
semantic: $(SEMANTIC_BINARY)
test-semantic: semantic
	@bash tests/semantic/run-tests.sh

$(SEMANTIC_BINARY): $(PARSER_GENERATED_C) $(PARSER_GENERATED_H) $(LEXER_GENERATED) src/ast.c src/ast.h src/inferencia.c src/inferencia.h src/main_semantico.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DANALISE_SEMANTICA $(PARSER_GENERATED_C) $(LEXER_GENERATED) src/ast.c src/inferencia.c src/main_semantico.c -o $@

all: parser

parser: $(PARSER_BINARY)

test: parser
	@bash tests/parser/run-tests.sh $(ISSUE)

test-ast: parser | $(BUILD_DIR)
	@$(CC) $(CFLAGS) -Isrc src/ast.c tests/ast/teste_criacao_nos.c -o $(BUILD_DIR)/teste_criacao_nos
	@$(CC) $(CFLAGS) -Isrc src/ast.c tests/ast/teste_adicionar_filho_nulo.c -o $(BUILD_DIR)/teste_adicionar_filho_nulo
	@$(CC) $(CFLAGS) -Isrc src/ast.c tests/ast/teste_saida_impressao.c -o $(BUILD_DIR)/teste_saida_impressao
	@$(CC) $(CFLAGS) -Isrc src/ast.c tests/ast/teste_impressao_arvore_vazia.c -o $(BUILD_DIR)/teste_impressao_arvore_vazia
	@$(BUILD_DIR)/teste_criacao_nos
	@$(BUILD_DIR)/teste_adicionar_filho_nulo
	@$(BUILD_DIR)/teste_saida_impressao
	@$(BUILD_DIR)/teste_impressao_arvore_vazia
	@bash tests/ast/run-tests.sh
	@echo "Todos os testes unitários da AST passaram com sucesso!"

$(PARSER_BINARY): $(PARSER_GENERATED_C) $(PARSER_GENERATED_H) $(LEXER_GENERATED) src/ast.c src/ast.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(PARSER_GENERATED_C) $(LEXER_GENERATED) src/ast.c -o $@

$(PARSER_GENERATED_C) $(PARSER_GENERATED_H): $(PARSER_SOURCE) | $(BUILD_DIR)
	$(BISON) -d -o $(PARSER_GENERATED_C) $(PARSER_SOURCE)

$(LEXER_GENERATED): $(LEXER_SOURCE) $(PARSER_GENERATED_H) | $(BUILD_DIR)
	$(FLEX) -o $(LEXER_GENERATED) $(LEXER_SOURCE)

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
