CC := gcc
FLEX := flex
BISON := bison

CFLAGS := -Wall -Wextra -Ibuild -Isrc -Isrc/ast -Isrc/semantic -Isrc/codegen
BUILD_DIR := build

LEXER_SOURCE := lexer/lexer.l
LEXER_GENERATED := $(BUILD_DIR)/lex.yy.c

PARSER_SOURCE := parser/parser.y
PARSER_GENERATED_C := $(BUILD_DIR)/parser.tab.c
PARSER_GENERATED_H := $(BUILD_DIR)/parser.tab.h
PARSER_BINARY := $(BUILD_DIR)/parser

SEMANTIC_BINARY := $(BUILD_DIR)/semantic
CODEGEN_TEST_BINARY := $(BUILD_DIR)/teste_codegen_expressoes
CODEGEN_IO_BINARY := $(BUILD_DIR)/teste_io_unitario

.PHONY: all parser semantic test test-ast test-tabela test-semantic test-codegen clean

all: parser

parser: $(PARSER_BINARY)

semantic: $(SEMANTIC_BINARY)

$(SEMANTIC_BINARY): $(PARSER_GENERATED_C) $(PARSER_GENERATED_H) $(LEXER_GENERATED) src/ast/ast.c src/ast/ast.h src/semantic/inferencia.c src/semantic/inferencia.h src/semantic/main_semantico.c src/semantic/tabela.c src/semantic/tabela.h src/semantic/semantica.c src/semantic/semantica.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DANALISE_SEMANTICA $(PARSER_GENERATED_C) $(LEXER_GENERATED) src/ast/ast.c src/semantic/tabela.c src/semantic/semantica.c src/semantic/inferencia.c src/semantic/main_semantico.c -o $@

test: parser
	@bash tests/parser/run-tests.sh $(ISSUE)

test-ast: parser | $(BUILD_DIR)
	@$(CC) $(CFLAGS) src/ast/ast.c tests/ast/teste_criacao_nos.c -o $(BUILD_DIR)/teste_criacao_nos
	@$(CC) $(CFLAGS) src/ast/ast.c tests/ast/teste_adicionar_filho_nulo.c -o $(BUILD_DIR)/teste_adicionar_filho_nulo
	@$(CC) $(CFLAGS) src/ast/ast.c tests/ast/teste_saida_impressao.c -o $(BUILD_DIR)/teste_saida_impressao
	@$(CC) $(CFLAGS) src/ast/ast.c tests/ast/teste_impressao_arvore_vazia.c -o $(BUILD_DIR)/teste_impressao_arvore_vazia
	@$(BUILD_DIR)/teste_criacao_nos
	@$(BUILD_DIR)/teste_adicionar_filho_nulo
	@$(BUILD_DIR)/teste_saida_impressao
	@$(BUILD_DIR)/teste_impressao_arvore_vazia
	@bash tests/ast/run-tests.sh
	@echo "Todos os testes unitários da AST passaram com sucesso!"

test-tabela: | $(BUILD_DIR)
	@$(CC) $(CFLAGS) src/semantic/tabela.c tests/semantic/teste_pilha_escopos.c -o $(BUILD_DIR)/teste_pilha_escopos
	@$(CC) $(CFLAGS) src/semantic/tabela.c tests/semantic/teste_busca_apos_fechamento_escopo.c -o $(BUILD_DIR)/teste_busca_apos_fechamento_escopo
	@$(BUILD_DIR)/teste_pilha_escopos
	@$(BUILD_DIR)/teste_busca_apos_fechamento_escopo
	@echo "Todos os testes unitários da tabela de símbolos passaram com sucesso!"

test-semantic: $(PARSER_BINARY) semantic | $(BUILD_DIR)
	@$(CC) $(CFLAGS) src/semantic/tabela.c tests/semantic/teste_pilha_escopos.c -o $(BUILD_DIR)/teste_pilha_escopos
	@$(CC) $(CFLAGS) src/semantic/tabela.c tests/semantic/teste_busca_apos_fechamento_escopo.c -o $(BUILD_DIR)/teste_busca_apos_fechamento_escopo
	@$(CC) $(CFLAGS) src/ast/ast.c src/semantic/tabela.c src/semantic/semantica.c tests/semantic/teste_escopos_unitario.c -o $(BUILD_DIR)/teste_escopos_unitario
	@bash tests/semantic/run-tests.sh
	@echo "Todos os testes semânticos passaram com sucesso!"

test-codegen: parser $(CODEGEN_TEST_BINARY) $(CODEGEN_IO_BINARY) | $(BUILD_DIR)
	@$(BUILD_DIR)/teste_io_unitario
	@bash tests/codegen/run-tests.sh
	@echo "Todos os testes de geracao de codigo passaram com sucesso!"

$(PARSER_BINARY): $(PARSER_GENERATED_C) $(PARSER_GENERATED_H) $(LEXER_GENERATED) src/ast/ast.c src/ast/ast.h src/semantic/tabela.c src/semantic/tabela.h src/semantic/semantica.c src/semantic/semantica.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(PARSER_GENERATED_C) $(LEXER_GENERATED) src/ast/ast.c src/semantic/tabela.c src/semantic/semantica.c -o $@

$(PARSER_GENERATED_C) $(PARSER_GENERATED_H): $(PARSER_SOURCE) | $(BUILD_DIR)
	$(BISON) -d -o $(PARSER_GENERATED_C) $(PARSER_SOURCE)

$(LEXER_GENERATED): $(LEXER_SOURCE) $(PARSER_GENERATED_H) | $(BUILD_DIR)
	$(FLEX) -o $(LEXER_GENERATED) $(LEXER_SOURCE)

$(CODEGEN_TEST_BINARY): $(PARSER_GENERATED_C) $(PARSER_GENERATED_H) $(LEXER_GENERATED) src/ast/ast.c src/ast/ast.h src/semantic/tabela.c src/semantic/tabela.h src/semantic/semantica.c src/semantic/semantica.h src/codegen/codegen.h src/codegen/codegen_expr.c src/codegen/codegen_main.c tests/codegen/teste_codegen_expressoes.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DANALISE_SEMANTICA $(PARSER_GENERATED_C) $(LEXER_GENERATED) src/ast/ast.c src/semantic/tabela.c src/semantic/semantica.c src/codegen/codegen_expr.c src/codegen/codegen_main.c tests/codegen/teste_codegen_expressoes.c -o $@

$(CODEGEN_IO_BINARY): tests/codegen/teste_io_unitario.c src/codegen/codegen_main.c src/codegen/codegen_io.c src/ast/ast.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) tests/codegen/teste_io_unitario.c src/codegen/codegen_main.c src/codegen/codegen_io.c src/ast/ast.c -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
