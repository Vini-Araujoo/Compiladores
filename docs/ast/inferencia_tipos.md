# Inferência de Tipos (Issue 9.3)

## 1. Visão Geral
Este documento especifica o algoritmo de inferência e checagem de tipos da linguagem. A inferência é realizada percorrendo a Árvore de Sintaxe Abstrata (AST) das expressões de forma **bottom-up** (pós-ordem), calculando e anotando o campo `tipo_dado` em cada nó intermediário e folha.

## 2. Regras de Inferência e Promoção Numérica

### 2.1 Nós Folha
- **Literais Inteiros (`NO_LITERAL_INT`):** `TIPO_INT`.
- **Literais Flutuantes (`NO_LITERAL_DOUBLE`):** `TIPO_DOUBLE`.
- **Literais Booleanos (`NO_LITERAL_BOOL`):** `TIPO_BOOLEAN`.
- **Identificadores (`NO_IDENTIFICADOR`):** O tipo é retornado após consulta à Tabela de Símbolos (`buscarTipoSimboloMock` caso a tabela não esteja integrada).

### 2.2 Operadores Unários (`NO_OP_UNARIA`)
- **Negação Lógica (`!`):** O operando deve ser booleano, e o resultado é sempre `TIPO_BOOLEAN`.
- **Operadores Aritméticos Unários (`+`, `-`):** Preservam o tipo numérico do operando (`TIPO_DOUBLE` permanece `TIPO_DOUBLE`; tipos integrais resultam em no mínimo `TIPO_INT`).

### 2.3 Operadores Binários (`NO_OP_BINARIA`)
- **Operadores Relacionais e Lógicos (`<`, `<=`, `>`, `>=`, `==`, `!=`, `&&`, `||`):** Resultam sempre em `TIPO_BOOLEAN`.
- **Operadores Aritméticos (`+`, `-`, `*`, `/`, `%`):**
  * Aplicam a **promoção numérica estilo Java**:
    * Se qualquer um dos operandos for `TIPO_DOUBLE`, o tipo do nó resultante é anotado como `TIPO_DOUBLE`.
    * Caso contrário, se ambos forem válidos e integrais, o resultado é `TIPO_INT`.
  * Se algum operando tiver tipo `TIPO_INDEFINIDO` ou incompatível, o nó é anotado como `TIPO_INDEFINIDO`.

## 3. Estrutura de Invocação e Anotação
A função principal `TipoDado inferirTipoExpressao(NoAST *expr)` realiza a avaliação recursiva. Cada nó processado garante o preenchimento explícito do campo `expr->tipo_dado` antes de retornar o seu tipo ao nó pai.