# Visualização da Árvore Sintática Abstrata (Issue 6.4)

Este documento descreve o módulo de impressão formatada e recursiva da Árvore Sintática Abstrata (AST) do compilador Java-para-C, implementado em `src/ast.c`.

---

## 1. Visão Geral

O módulo de visualização (`imprimirAST`) tem como finalidade gerar uma representação gráfica legível e hierárquica da árvore gerada pelo parser no terminal. Essa ferramenta facilita a depuração, validação de regras sintáticas e o acompanhamento das árvores geradas por todas as construções da linguagem.

---

## 2. Interface e Arquitetura

A interface pública está definida em `src/ast.h`:

```c
void imprimirAST(const NoAST *raiz);
```

### 2.1 Algoritmo Recursivo (`imprimirASTRec`)
A impressão percorre a árvore em pré-ordem utilizando caracteres de desenho de caixas (*box-drawing characters* UTF-8) para indicar a hierarquia de forma visual:
* `├── `: Anexado a nós intermediários de uma mesma ramificação.
* `└── `: Anexado ao último filho de um nó.
* `│   `: Prefixo vertical indicando continuidade dos nós irmãos nos níveis superiores.
* Rótulos contextuais (`rotuloFilho`): Identificam segmentos específicos de nós complexos, como partes do laço `NO_FOR` (`inicializacao`, `condicao`, `incremento`, `bloco`) e do comando `NO_SWITCH` (`seletor`, `casos`, `default`).

---

## 3. Tratamento Defensivo contra Nulos

Em compiladores desenvolvidos em equipe, nós podem conter filhos não preenchidos ou cláusulas opcionais ausentes (por exemplo, partes vazias em `for(;;)` ou um `switch` sem `default`).
* Se qualquer ponteiro de nó for `NULL`, a rotina imprime `<vazio>` e retorna de forma segura.
* Se a própria raiz passada para `imprimirAST` for `NULL`, ela emite `<vazio>` sem gerar *Segmentation Fault*.

---

## 4. Exibição de Nós e Metadados

Cada nó exibe seu tipo sintático seguido de informações contextuais específicas entre parênteses:
1. **Identificadores e Nomes:**
   * Classes: `NO_CLASSE (Nome)`
   * Métodos: `NO_METODO (nome)`
   * Parâmetros: `NO_PARAMETRO (arg)`
   * Identificadores de variáveis: `NO_IDENTIFICADOR (x)`
2. **Literais:**
   * Inteiros: `NO_LITERAL_INT (valor)`
   * Ponto flutuante: `NO_LITERAL_DOUBLE (valor)` / `NO_LITERAL_FLOAT (valor)`
   * Strings: `NO_LITERAL_STRING ("texto")`
   * Caracteres: `NO_LITERAL_CHAR ('c')`
   * Booleanos: `NO_LITERAL_BOOL (true/false)`
3. **Operadores:**
   * Operações binárias e unárias: `NO_OP_BINARIA (+)` / `NO_OP_UNARIA (!)`

---

## 5. Exemplos de Saída Formatada

### 5.1 Classe com Método `main`

Para o código:
```java
public class Main {
    public static void main(String[] args) {}
}
```

A saída formatada da AST é:
```text
NO_CLASSE (Main)
└── filho[0]: NO_METODO (main)
    ├── filho[0]: NO_PARAMETROS
    │   └── filho[0]: NO_PARAMETRO (args)
    └── filho[1]: NO_BLOCO
```

### 5.2 Método com Múltiplos Parâmetros

Para o código:
```java
public class Calculadora {
    double calcular(int a, double b, boolean flag) {}
}
```

A saída formatada é:
```text
NO_CLASSE (Calculadora)
└── filho[0]: NO_METODO (calcular)
    ├── filho[0]: NO_PARAMETROS
    │   ├── filho[0]: NO_PARAMETRO (a)
    │   ├── filho[1]: NO_PARAMETRO (b)
    │   └── filho[2]: NO_PARAMETRO (flag)
    └── filho[1]: NO_BLOCO
```

### 5.3 Árvore Vazia (Raiz Nula)

Chamando `imprimirAST(NULL)`:
```text
<vazio>
```

---

## 6. Testes Unitários

A suíte de testes da Issue 6.4 está integrada ao `make test-ast`:
* **`tests/ast/teste_saida_impressao.c`:** Monta uma AST estruturada em memória, captura a saída de `imprimirAST` e valida via asserções de string que os ramos, prefixos e identificadores são emitidos conforme o padrão esperado.
* **`tests/ast/teste_impressao_arvore_vazia.c`:** Invoca `imprimirAST(NULL)` para garantir execução defensiva segura com código de saída 0.

Execução:
```bash
make test-ast
```
