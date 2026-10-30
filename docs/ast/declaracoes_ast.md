# AST de Variáveis e Atribuições (Issue 8.2)

Este documento descreve as especificações técnicas, decisões de projeto e implementação dos nós da Árvore Sintática Abstrata (AST) para declarações de variáveis (locais e membros de classe) e instruções de atribuição (simples e compostas).

---

## 1. Visão Geral

A Issue 8.2 conecta as regras sintáticas `declaracao_variavel`, `declaracao_local` e `atribuicao` à construção dos nós da AST.

Construções tratadas:
- Declarações de variáveis em escopo de classe (atributos/campos).
- Declarações de variáveis em escopo de métodos/blocos (variáveis locais).
- Declarações múltiplas na mesma instrução (ex.: `int a = 1, b, c = 3;`).
- Declarações com e sem inicialização encadeada.
- Atribuições simples (`=`) e compostas (`+=`, `-=`).

---

## 2. Estrutura dos Nós

### 2.1 Nó `NO_DECL_VARIAVEL`

Representa uma declaração individual de variável:

| Campo | Descrição |
| --- | --- |
| `tipo_no` | `NO_DECL_VARIAVEL` (com alias `#define NO_VAR_DECL NO_DECL_VARIAVEL`) |
| `tipo_dado` | Enum `TipoDado` correspondente ao tipo declarado (`TIPO_INT`, `TIPO_DOUBLE`, `TIPO_BOOLEAN`, `TIPO_BYTE`, `TIPO_SHORT`, `TIPO_LONG`, `TIPO_FLOAT`, `TIPO_CHAR`, `TIPO_STRING`, etc.) |
| `valor.valor_str` | Identificador (nome) da variável (string alocada dinamicamente) |
| `linha` | Linha correspondente no arquivo fonte Java |
| `filhos[0]` | *(Opcional)* Expressão de inicialização. Quando não houver inicialização, possui 0 filhos (ou ponteiro `NULL` seguro) |

### 2.2 Nó `NO_ATRIBUICAO`

Representa uma instrução de atribuição:

| Campo | Descrição |
| --- | --- |
| `tipo_no` | `NO_ATRIBUICAO` |
| `valor.operador` | Operador registrado: `"="` (atribuição simples), `"+="` ou `"-="` (atribuições compostas) |
| `linha` | Linha correspondente no arquivo fonte Java |
| `filhos[0]` | Alvo da atribuição / L-value (`NO_IDENTIFICADOR` ou `NO_ACESSO_ARRAY`) |
| `filhos[1]` | Expressão atribuída / R-value |

---

## 3. Declarações Múltiplas

Em Java, é comum declarar mais de uma variável sob o mesmo especificador de tipo, podendo algumas ter valor inicial e outras não:

```java
int a = 1, b, c = 3;
```

### Estratégia de Desmembramento
A regra sintática `lista_declaracoes` acumula cada `item_declaracao` em um contêiner temporário. Ao finalizar o reconhecimento da instrução em `declaracao_variavel` ou `declaracao_local`, o tipo especificado (`tipo`) é propagado para cada um dos nós.

Ao anexar ao nó pai:
- Em **escopo de classe** (`corpo_classe`), cada declaração individual é inserida diretamente como filho do nó `NO_CLASSE`.
- Em **escopo local** (`lista_comandos`), cada declaração individual é inserida diretamente como filho do nó `NO_BLOCO`.
- O contêiner temporário é liberado, resultando em uma representação homogênea e plana na AST.

---

## 4. Atribuições Compostas (`+=`, `-=`)

As atribuições compostas registram explicitamente o operador utilizado no campo `valor.operador` da `struct NoAST`:
- Para `=`, armazena `"="`.
- Para `+=`, armazena `"+="`.
- Para `-=`, armazena `"-="`.

Na impressão formatada da árvore (`imprimirAST`):
- Atribuições simples imprimem `NO_ATRIBUICAO`.
- Atribuições compostas imprimem `NO_ATRIBUICAO (+=)` e `NO_ATRIBUICAO (-=)`.

---

## 5. Decisões Técnicas de Projeto

1. **Interdependência e Inicializações Opcionais / Nulas:**
   Declarações sem valor inicial (ex.: `int b;`) possuem 0 nós filhos. Caso uma inicialização nula (`NULL`) seja adicionada explicitamente, as rotinas de visualização e desalocação em `src/ast.c` tratam o nó sem causar falhas de segmentação.
2. **Gerenciamento de Memória (`liberarAST`):**
   Os identificadores armazenados em `valor.valor_str` no nó `NO_DECL_VARIAVEL` são liberados recursivamente pela função `liberarAST`, prevenindo vazamentos de memória.
3. **Compatibilidade de Nomes:**
   Para garantir compatibilidade com código preexistente no repositório, o enum `TipoNoAST` define `NO_DECL_VARIAVEL` com o macro `#define NO_VAR_DECL NO_DECL_VARIAVEL`.
4. **Ausência de Conflitos Sintáticos:**
   A integração das regras preserva a integridade da gramática LALR(1) no Bison, sem nenhum conflito *Shift/Reduce* ou *Reduce/Reduce*.

---

## 6. Exemplos Visuais da AST

### 6.1 Declarações Múltiplas e Campos de Classe

Código Java:
```java
class Exemplo {
    int x = 10, y, z = 20;

    void testar() {
        int a = 1, b, c = 3;
    }
}
```

Árvore gerada:
```text
NO_CLASSE (Exemplo)
├── filho[0]: NO_DECL_VARIAVEL (x)
│   └── filho[0]: NO_LITERAL_INT (10)
├── filho[1]: NO_DECL_VARIAVEL (y)
├── filho[2]: NO_DECL_VARIAVEL (z)
│   └── filho[0]: NO_LITERAL_INT (20)
└── filho[3]: NO_METODO (testar)
    ├── filho[0]: NO_PARAMETROS
    └── filho[1]: NO_BLOCO
        ├── filho[0]: NO_DECL_VARIAVEL (a)
        │   └── filho[0]: NO_LITERAL_INT (1)
        ├── filho[1]: NO_DECL_VARIAVEL (b)
        └── filho[2]: NO_DECL_VARIAVEL (c)
            └── filho[0]: NO_LITERAL_INT (3)
```

### 6.2 Atribuições Simples e Compostas

Código Java:
```java
class ExemploAtribuicao {
    void executar() {
        x = 10;
        contador += 1;
        saldo -= 5;
    }
}
```

Árvore gerada:
```text
NO_CLASSE (ExemploAtribuicao)
└── filho[0]: NO_METODO (executar)
    ├── filho[0]: NO_PARAMETROS
    └── filho[1]: NO_BLOCO
        ├── filho[0]: NO_ATRIBUICAO
        │   ├── filho[0]: NO_IDENTIFICADOR (x)
        │   └── filho[1]: NO_LITERAL_INT (10)
        ├── filho[1]: NO_ATRIBUICAO (+=)
        │   ├── filho[0]: NO_IDENTIFICADOR (contador)
        │   └── filho[1]: NO_LITERAL_INT (1)
        └── filho[2]: NO_ATRIBUICAO (-=)
            ├── filho[0]: NO_IDENTIFICADOR (saldo)
            └── filho[1]: NO_LITERAL_INT (5)
```

---

## 7. Testes Automatizados

### Testes Adicionados
* **`tests/ast/valido_declaracao_multipla.java`**: Validação positiva com declarações múltiplas, campos de classe e variáveis locais (inteiros, doubles e booleanos).
* **`tests/ast/valido_atribuicao_simples.java`**: Validação positiva de atribuição simples (`=`) e compostas (`+=`, `-=`).
* **`tests/ast/invalido_declaracao_sem_tipo.java`**: Validação negativa que garante a rejeição de declaração sem especificação de tipo com mensagem informativa e número de linha.
* **`tests/ast/teste_criacao_nos.c`**: Testes unitários em C para criação, encadeamento e liberação de `NO_DECL_VARIAVEL` (com inicialização, sem inicialização e com filho nulo) e `NO_ATRIBUICAO` (`=`, `+=`, `-=`).
* **`tests/ast/teste_saida_impressao.c`**: Testes unitários em C validando a formatação impressa de `NO_DECL_VARIAVEL` e operadores compostos de atribuição.

### Como Executar os Testes
```bash
make test-ast
make test
```
