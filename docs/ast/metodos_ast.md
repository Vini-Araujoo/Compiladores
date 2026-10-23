# Representação de Métodos e Parâmetros na AST (Issue 6.3)

Este documento descreve a especificação técnica e as decisões de projeto da representação de métodos, parâmetros formais e corpos de métodos na Árvore Sintática Abstrata (AST) do compilador Java-para-C.

---

## 1. Visão Geral

A declaração de método (`declaracao_metodo`) representa as funções contidas dentro de uma classe Java (`NO_CLASSE`). Cada método é representado por um nó `NO_METODO` contendo:
* **Identificador:** O nome do método armazenado no campo `valor.valor_str`.
* **Tipo de retorno:** O tipo associado (`TipoDado`) armazenado em `tipo_dado` (ex: `TIPO_VOID`, `TIPO_INT`, `TIPO_DOUBLE`).
* **Dois filhos diretos:**
  * `filhos[0]`: Nó agregador de parâmetros (`NO_PARAMETROS`).
  * `filhos[1]`: Nó do corpo do método (`NO_BLOCO`).

---

## 2. Estrutura dos Nós

### 2.1 Nó `NO_METODO`
Representa a assinatura e o cabeçalho do método.
* `tipo_no`: `NO_METODO`
* `tipo_dado`: Enum `TipoDado` correspondente ao retorno (`TIPO_VOID` ou tipo primitivo mapeado pela regra `tipo`).
* `valor.valor_str`: String contendo o identificador do método (alocado dinamicamente pelo lexer).
* `filhos[0]`: `NO_PARAMETROS` (obrigatório, contendo 0 ou mais parâmetros).
* `filhos[1]`: `NO_BLOCO` (corpo de comandos do método).

### 2.2 Nó `NO_PARAMETROS`
Contêiner encadeador que agrupa os parâmetros formais do método.
* `tipo_no`: `NO_PARAMETROS`
* `num_filhos`: Quantidade de argumentos passados na assinatura.
* `filhos[i]`: Instâncias de `NO_PARAMETRO` para cada argumento formal.

### 2.3 Nó `NO_PARAMETRO`
Representa um argumento individual da lista de parâmetros.
* `tipo_no`: `NO_PARAMETRO`
* `tipo_dado`: Tipo do argumento (`TIPO_INT`, `TIPO_DOUBLE`, `TIPO_ARRAY`, etc.).
* `valor.valor_str`: Identificador do parâmetro (ex: `"args"`, `"x"`).

---

## 3. Decisões Técnicas de Projeto

1. **Modificadores (`public`, `static`, `final`, etc.):**
   Em conformidade com a decisão documentada na Issue 6.2 para classes (`docs/ast/estrutura_ast.md`), modificadores são validados sintaticamente pela regra `modificadores`, mas **não geram nós adicionais na AST**, uma vez que em C funções de unidade de tradução única não utilizam modificadores de visibilidade orientados a objeto.

2. **Parâmetros do Tipo Array (`String[] args`):**
   Para suportar a assinatura canônica do método de entrada `public static void main(String[] args)`, a regra gramatical `tipo '[' ']' ID` associa ao nó `NO_PARAMETRO` o tipo de dado `TIPO_ARRAY`. Na tradução subsequente para C, esse parâmetro mapeia diretamente para os argumentos padrão `int argc, char *argv[]`.

3. **Gerenciamento de Memória:**
   Os identificadores armazenados em `valor.valor_str` para `NO_METODO` e `NO_PARAMETRO` têm sua propriedade transferida para a AST. A função `liberarAST` em `src/ast.c` desaloca recursivamente as strings desses nós ao término da execução.

---

## 4. Diagramas da AST

### 4.1 Assinatura Canônica: `public static void main(String[] args)`

```text
NO_CLASSE (Main)
└── filho[0]: NO_METODO (main)
    ├── filho[0]: NO_PARAMETROS
    │   └── filho[0]: NO_PARAMETRO (args)
    └── filho[1]: NO_BLOCO
```

### 4.2 Método com Múltiplos Parâmetros: `double calcular(int a, double b, boolean flag)`

```text
NO_CLASSE (Calculadora)
└── filho[0]: NO_METODO (calcular)
    ├── filho[0]: NO_PARAMETROS
    │   ├── filho[0]: NO_PARAMETRO (a)
    │   ├── filho[1]: NO_PARAMETRO (b)
    │   └── filho[2]: NO_PARAMETRO (flag)
    └── filho[1]: NO_BLOCO
```

### 4.3 Método sem Parâmetros: `void executar()`

```text
NO_CLASSE (App)
└── filho[0]: NO_METODO (executar)
    ├── filho[0]: NO_PARAMETROS
    └── filho[1]: NO_BLOCO
```

---

## 5. Testes

Os testes da Issue 6.3 estão localizados em `tests/ast/` e integrados ao runner `tests/ast/run-tests.sh`:
* **`valido_metodo_main.java`**: Validação positiva da assinatura `main` com `String[] args`.
* **`valido_metodo_multiplos_params.java`**: Validação de método com retorno e múltiplos parâmetros encadeados.
* **`invalido_metodo_sem_parenteses.java`**: Validação negativa que garante rejeição com código de erro e indicação de linha caso parênteses sejam omitidos.
* **`teste_criacao_nos.c`**: Validação unitária em C da criação e liberação de `NO_PARAMETROS` e `NO_PARAMETRO`.

Execução:
```bash
make test-ast
```
