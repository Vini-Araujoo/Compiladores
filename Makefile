CC := gcc
FLEX := flex
BISON := bison

CFLAGS := -Wall -Wextra -I. -Ilexer
BUILD_DIR := build

LEXER_SOURCE := lexer/lexer.l
TOKEN_HEADER := lexer/enum/tokens.h
LEXER_GENERATED := $(BUILD_DIR)/lex.yy.c
LEXER_BINARY := $(BUILD_DIR)/lexico

PARSER_SOURCE := paser/paser.y
PARSER_GENERATED_C := $(BUILD_DIR)/paser.tab.c
PARSER_GENERATED_H := $(BUILD_DIR)/paser.tab.h
PARSER_OBJECT := $(BUILD_DIR)/paser.tab.o

.PHONY: all lexer parser clean

all: lexer parser

lexer: $(LEXER_BINARY)

$(LEXER_BINARY): $(LEXER_SOURCE) $(TOKEN_HEADER) | $(BUILD_DIR)
	$(FLEX) -o $(LEXER_GENERATED) $(LEXER_SOURCE)
	$(CC) $(CFLAGS) $(LEXER_GENERATED) -o $@

parser: $(PARSER_OBJECT)

$(PARSER_GENERATED_C) $(PARSER_GENERATED_H): $(PARSER_SOURCE) | $(BUILD_DIR)
	$(BISON) -d -o $(PARSER_GENERATED_C) $(PARSER_SOURCE)

$(PARSER_OBJECT): $(PARSER_GENERATED_C) $(PARSER_GENERATED_H)
	$(CC) $(CFLAGS) -c $(PARSER_GENERATED_C) -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)