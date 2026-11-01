# Análise Semântica do Modificador Final (Issue 8.4)

Documentação técnica da implementação da verificação de imutabilidade e restrições de constantes marcadas com o modificador `final` no compilador Java-para-C.

---

## 1. Identificação da Issue

- **Responsável:** Vini-Araujoo
- **Branch:** `feat/issue-8.4-semantica-final`
- **Módulo:** Análise Semântica de Constantes
- **Status:** Concluída

---

## 2. Visão Geral

Na linguagem Java, variáveis marcadas com a palavra-chave `final` são constantes que recebem um valor inicial e tornam-se imutáveis após essa definição. O objetivo desta alteração é:
1. Registrar a flag `is_final` (ou `eh_final`) no símbolo correspondente durante o cadastro na Tabela de Símbolos (`NO_DECL_VARIAVEL`).
2. Permitir normalmente a atribuição de valor no momento da inicialização/declaração da constante.
3. Interceptar e emitir erro semântico impeditivo diante de qualquer tentativa de reatribuição (`NO_ATRIBUICAO`) ou mutação por incremento/decremento (`++`, `--`).

---

## 3. Arquitetura e Fluxo de Dados

### 3.1 Propagação dos Modificadores da Gramática para a AST

No parser ([parser.y](file:///c:/Users/Vinícius/Documents/Compiladores/parser/parser.y)):
- As regras sintáticas `modificadores`, `modificador` e `modificador_variavel` foram tipadas como `<mods>` (`Modificadores`).
- Quando o token `TOKEN_FINAL` é encontrado, as flags `eh_final` e `is_final` são ativadas em `Modificadores`.
- Nas regras `declaracao_variavel` (campos da classe) e `declaracao_local` (variáveis locais), a estrutura `mods` é repassada para cada nó `NO_DECL_VARIAVEL` filho em `lista_declaracoes`.

### 3.2 Registro na Tabela de Símbolos

No módulo de gerenciamento de símbolos ([tabela.h](file:///c:/Users/Vinícius/Documents/Compiladores/src/tabela.h) e [tabela.c](file:///c:/Users/Vinícius/Documents/Compiladores/src/tabela.c)):
- A struct `Simbolo` armazena `int is_final;` (com compatibilidade via alias `#define is_final eh_final`).
- Ao invocar `inserirSimbolo(nome, tipo, mods, linha)`, o compilador grava `novo->eh_final = mods.eh_final;`.

### 3.3 Verificação Semântica de Imutabilidade

Na travessia da AST em [semantica.c](file:///c:/Users/Vinícius/Documents/Compiladores/src/semantica.c):
1. **Inicialização na Declaração (`NO_DECL_VARIAVEL`):**
   - A expressão inicializadora é avaliada como filha do nó de declaração, antes do cadastro do símbolo. Não constitui nó `NO_ATRIBUICAO`, sendo aceita naturalmente sem conflito.
2. **Reatribuição (`NO_ATRIBUICAO`):**
   - Ao inspecionar o nó de atribuição (`=` ou atribuição composta `+=`, `-=`), verifica-se se o filho esquerdo (`filhos[0]`) é um `NO_IDENTIFICADOR` cujo símbolo possui `s->is_final == 1`.
   - Caso positivo, bloqueia a compilação e emite erro semântico com a linha.
3. **Incremento e Decremento (`++`, `--`):**
   - Inspecionados tanto nos nós dedicados de comando (`NO_INCREMENTO`, `NO_DECREMENTO`, `NO_INCREMENTO_ANTES`, `NO_DECREMENTO_ANTES`) quanto em expressões unárias (`NO_OP_UNARIA`).
   - Se o identificador alvo possuir `s->is_final == 1`, a mutação é rejeitada com erro semântico.

---

## 4. Mensagens e Formato de Erros Semânticos

As mensagens seguem o padrão padronizado do compilador:

### 4.1 Tentativa de Reatribuição
```text
Erro semantico na linha <linha>: variavel final '<nome>' nao pode ser reatribuida.
```

### 4.2 Tentativa de Incremento
```text
Erro semantico na linha <linha>: variavel final '<nome>' nao pode ser incrementada.
```

### 4.3 Tentativa de Decremento
```text
Erro semantico na linha <linha>: variavel final '<nome>' nao pode ser decrementada.
```

---

## 5. Testes e Validação

### 5.1 Testes Unitários em C (`tests/semantic/teste_escopos_unitario.c`)
- **Cenário 6 (`teste_constante_final_valida`):** criação de constante `final int LIMITE = 50;` com inicialização e uso em expressão aritmética. Retorna sucesso (`1`) e valida que `simbolo->is_final == 1`.
- **Cenário 7 (`teste_mutacao_constante_final_invalida`):**
  - Caso A: tentativa de reatribuição (`MAX = 20;`). Valida rejeição (`0`).
  - Caso B: tentativa de incremento (`CONTADOR++;`). Valida rejeição (`0`).

### 5.2 Casos de Teste em Java (`tests/semantic/`)
- [valido_constante_final.java](file:///c:/Users/Vinícius/Documents/Compiladores/tests/semantic/valido_constante_final.java):
  - Declara constante global de classe `final int LIMITE_GLOBAL = 1000;`.
  - Declara constantes locais `final int MAX = 100;` e `final double PI = 3.14159;`.
  - Utiliza as constantes em operações sem alterá-las.
  - **Resultado:** Aprovado com código de saída `0`.
- [invalido_reatribuicao_final.java](file:///c:/Users/Vinícius/Documents/Compiladores/tests/semantic/invalido_reatribuicao_final.java):
  - Declara `final int MAX = 100;` e tenta reatribuir `MAX = 200;` na linha 4.
  - **Resultado:** Rejeitado com código diferente de zero e mensagem `Erro semantico na linha 4: variavel final 'MAX' nao pode ser reatribuida.`.
- [invalido_incremento_final.java](file:///c:/Users/Vinícius/Documents/Compiladores/tests/semantic/invalido_incremento_final.java):
  - Declara `final int CONTADOR = 0;` e tenta executar `CONTADOR++;` na linha 4.
  - **Resultado:** Rejeitado com código diferente de zero e mensagem `Erro semantico na linha 4: variavel final 'CONTADOR' nao pode ser incrementada.`.

### 5.3 Execução das Suítes

```bash
# Executa todos os testes de semântica e escopo (Issues 8.1, 8.3 e 8.4):
make test-semantic

# Executa testes sintáticos da AST:
make test-ast

# Executa testes gerais do parser:
make test
```
