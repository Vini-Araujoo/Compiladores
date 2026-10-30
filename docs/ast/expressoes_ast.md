# Expressões Aritméticas na AST (Issue 9.1)

Especificação e decisões técnicas da construção da AST para expressões aritméticas (`+`, `-`, `*`, `/`, `%`) e seus operandos folha.

---

## 1. Visão Geral

Em `parser/parser.y`, as ações de `expressao_sintatica` passaram a devolver um `NoAST*`. A hierarquia da árvore não é codificada nas ações, ela vem das declarações de precedência do Bison (`%left`), que definem a ordem das reduções.

```
%left   TOKEN_MAIS TOKEN_MENOS                 /* + -    (menor) */
%left   TOKEN_VEZES TOKEN_DIV TOKEN_MOD        /* * / %  (maior) */
```

Operadores de mesma precedência são associativos à esquerda: `10 - 4 - 3` é `(10 - 4) - 3`.

---

## 2. Estrutura dos Nós

### 2.1 `NO_OP_BINARIA`
* `valor.operador`: `"+"`, `"-"`, `"*"`, `"/"` ou `"%"`.
* `filhos[0]`: operando esquerdo.
* `filhos[1]`: operando direito.

### 2.2 Folhas

| Nó | Token | Campo |
|---|---|---|
| `NO_IDENTIFICADOR` | `ID` | `valor.valor_str` (nome) |
| `NO_LITERAL_INT` | `NUMERO` | `valor.valor_int` |
| `NO_LITERAL_DOUBLE` | `NUMERO_DOUBLE` | `valor.valor_double` |

> A issue cita `NO_OP_BINARIO` e `NO_ID`; os equivalentes já existentes em `src/tipos.h` (`NO_OP_BINARIA` e `NO_IDENTIFICADOR`), e graças a isso foram reaproveitados.

### 2.3 Parênteses
`( expr )` devolve diretamente o nó de `expr`. Parênteses não geram nó, só forçam a redução da subexpressão antes do operador externo.

---

## 3. Decisões Técnicas

1. **Precedência via Bison:** mantida a gramática única com `%left`, sem criar `termo`/`fator`. Resultado: nenhum conflito Shift/Reduce ou Reduce/Reduce.
2. **`%` com a precedência de `*` e `/`**, como em Java. O token não existia no lexer.
3. **Valores no `yylval`:** o lexer converte com `strtol`/`strtod`, não há alocação a liberar.
4. **Stubs genéricos (`NO_EXPRESSAO`):** formas que pertencem a outras issues continuam sem nó próprio e devolvem um `NO_EXPRESSAO` genérico: relacionais, lógicos, unários, `true`/`false`, chamadas de método, `new` e leitura de `Scanner`. Nos operadores, os operandos viram filhos. Acessos a array continuam devolvendo o `NO_ACESSO_ARRAY` que já existia. Assim `a + foo() * 2` já monta a árvore aritmética.
5. **Linha:** `NO_OP_BINARIA` herda a linha do operando esquerdo e folhas usam `yylineno` (convenção já usada em `cria_no_instancia_id`).

---

## 4. Exemplos de Árvores

### `2 + 3 * 4`: multiplicação é subárvore da adição
```text
NO_OP_BINARIA (+)
├── filho[0]: NO_LITERAL_INT (2)
└── filho[1]: NO_OP_BINARIA (*)
    ├── filho[0]: NO_LITERAL_INT (3)
    └── filho[1]: NO_LITERAL_INT (4)
```

### `(2 + 3) * 4`: parênteses invertem a hierarquia
```text
NO_OP_BINARIA (*)
├── filho[0]: NO_OP_BINARIA (+)
│   ├── filho[0]: NO_LITERAL_INT (2)
│   └── filho[1]: NO_LITERAL_INT (3)
└── filho[1]: NO_LITERAL_INT (4)
```

### `a + 2 * b`: identificadores e literais
```text
NO_OP_BINARIA (+)
├── filho[0]: NO_IDENTIFICADOR (a)
└── filho[1]: NO_OP_BINARIA (*)
    ├── filho[0]: NO_LITERAL_INT (2)
    └── filho[1]: NO_IDENTIFICADOR (b)
```

### `10 - 4 - 3`: associatividade à esquerda
```text
NO_OP_BINARIA (-)
├── filho[0]: NO_OP_BINARIA (-)
│   ├── filho[0]: NO_LITERAL_INT (10)
│   └── filho[1]: NO_LITERAL_INT (4)
└── filho[1]: NO_LITERAL_INT (3)
```

### `8 / 2 % 3` e `1.5 + a * 2.25`
```text
NO_OP_BINARIA (%)                    NO_OP_BINARIA (+)
├── filho[0]: NO_OP_BINARIA (/)      ├── filho[0]: NO_LITERAL_DOUBLE (1.5)
│   ├── filho[0]: NO_LITERAL_INT (8) └── filho[1]: NO_OP_BINARIA (*)
│   └── filho[1]: NO_LITERAL_INT (2)     ├── filho[0]: NO_IDENTIFICADOR (a)
└── filho[1]: NO_LITERAL_INT (3)         └── filho[1]: NO_LITERAL_DOUBLE (2.25)
```

---

## 5. Testes

Em `tests/ast/`, executados por `tests/ast/run-tests.sh`. Os positivos comparam a **árvore impressa completa** com o arquivo em `tests/ast/esperado/`.

| Teste | Tipo | Verifica |
|---|---|---|
| `valido_precedencia_aritmetica.java` | positivo | `2 + 3 * 4`, `a + 2 * b`, `10 - 4 - 3`, `8 / 2 % 3`, literais `double` |
| `valido_expressao_com_parenteses.java` | positivo | `(2 + 3) * 4`, `(a + b) / (c - d)`, `2 * (3 + 4) - 5` |
| `invalido_operador_sem_operando.java` | negativo | `a + ;` rejeitado com `Erro sintatico na linha 3` |

```bash
make test-ast
```