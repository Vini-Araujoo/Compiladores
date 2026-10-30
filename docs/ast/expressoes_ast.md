# Expressões na AST (Issues 9.1 e 9.2)

Especificação e decisões técnicas da construção da AST para expressões aritméticas (`+`, `-`, `*`, `/`, `%`) e seus operandos folha (Issue 9.1), e para comparações relacionais (`==`, `!=`, `<`, `>`, `<=`, `>=`), operadores lógicos (`&&`, `||`), unários (`!`, `-`, `++`, `--`) e literais booleanos (Issue 9.2).

---

## 1. Visão Geral

Em `parser/parser.y`, as ações de `expressao_sintatica` passaram a devolver um `NoAST*`. A hierarquia da árvore não é codificada nas ações, ela vem das declarações de precedência do Bison (`%left`), que definem a ordem das reduções.

```
%left   TOKEN_OU                                                      /* ||       (menor) */
%left   TOKEN_E                                                       /* &&               */
%left   TOKEN_IGUAL TOKEN_DIFERENTE                                   /* == !=            */
%left   TOKEN_MENOR TOKEN_MAIOR TOKEN_MENOR_IGUAL TOKEN_MAIOR_IGUAL   /* < > <= >=        */
%left   TOKEN_MAIS TOKEN_MENOS                                        /* + -              */
%left   TOKEN_VEZES TOKEN_DIV TOKEN_MOD                               /* * / %            */
%precedence TOKEN_NEGACAO UMINUS                                      /* ! - (prefixos)   */
%precedence TOKEN_INCREMENTO TOKEN_DECREMENTO                         /* ++ --    (maior) */
```

Parênteses têm a maior precedência, pois a regra `( expr )` fecha a subexpressão antes de qualquer operador externo. A ordem resultante é parênteses > unários > relacionais > lógicos.

Operadores de mesma precedência são associativos à esquerda: `10 - 4 - 3` é `(10 - 4) - 3`.

---

## 2. Estrutura dos Nós

### 2.1 `NO_OP_BINARIA`
* `valor.operador`: aritméticos `"+"`, `"-"`, `"*"`, `"/"`, `"%"`; relacionais `"=="`, `"!="`, `"<"`, `">"`, `"<="`, `">="`; lógicos `"&&"`, `"||"`.
* `filhos[0]`: operando esquerdo.
* `filhos[1]`: operando direito.

### 2.2 Folhas

| Nó | Token | Campo |
|---|---|---|
| `NO_IDENTIFICADOR` | `ID` | `valor.valor_str` (nome) |
| `NO_LITERAL_INT` | `NUMERO` | `valor.valor_int` |
| `NO_LITERAL_DOUBLE` | `NUMERO_DOUBLE` | `valor.valor_double` |
| `NO_LITERAL_BOOL` | `true` / `false` | `valor.valor_int` (1 ou 0) |

> A issue cita `NO_OP_BINARIO` e `NO_ID`; os equivalentes já existentes em `src/tipos.h` (`NO_OP_BINARIA` e `NO_IDENTIFICADOR`), e graças a isso foram reaproveitados.

### 2.3 Parênteses
`( expr )` devolve diretamente o nó de `expr`. Parênteses não geram nó, só forçam a redução da subexpressão antes do operador externo.

### 2.4 `NO_OP_UNARIA`
* `valor.operador`: `"!"`, `"-"`, `"++"` ou `"--"`.
* `filhos[0]`: operando.
* `posfixo` (campo de `NoAST`): `1` para operador pós-fixado (`x++`, `x--`) e `0` para pré-fixado (`!x`, `-x`, `++x`, `--x`).
* Na impressão da AST, o pré-fixado aparece como `NO_OP_UNARIA (++)` e o pós-fixado como `NO_OP_UNARIA (++, posfixo)`.

---

## 3. Decisões Técnicas

1. **Precedência via Bison:** mantida a gramática única com `%left`, sem criar `termo`/`fator`. Resultado: nenhum conflito Shift/Reduce ou Reduce/Reduce.
2. **`%` com a precedência de `*` e `/`**, como em Java. O token não existia no lexer.
3. **Valores no `yylval`:** o lexer converte com `strtol`/`strtod`, não há alocação a liberar.
4. **Stubs genéricos (`NO_EXPRESSAO`):** formas que pertencem a outras issues continuam sem nó próprio e devolvem um `NO_EXPRESSAO` genérico: chamadas de método e leitura de `Scanner`. Acessos e instanciação de array usam os nós da Issue 10.1. Assim `a + foo() * 2` já monta a árvore aritmética.
5. **Linha:** `NO_OP_BINARIA` herda a linha do operando esquerdo e folhas usam `yylineno` (convenção já usada em `cria_no_instancia_id`).

6. **Um único nó por categoria (Issue 9.2):** todos os binários relacionais e lógicos usam `NO_OP_BINARIA`, e todos os unários usam `NO_OP_UNARIA`, sem um tipo de nó por operador. O operador fica em `valor.operador`, como string, o mesmo formato já usado para os aritméticos na Issue 9.1. O texto da issue cita o código do token (`TOKEN_OU`, ...); `valor.operador` é um `char[4]` em `ValorLiteral`, e manter a string deixa todos os binários uniformes.
7. **Direção do unário em `NoAST.posfixo`:** `ValorLiteral` é uma union, então o operador e a direção não podem dividir o mesmo campo. Foi adicionado `int posfixo` em `NoAST`, inicializado com `0` em `criarNo`.
8. **`++` e `--` em expressões:** `x++`, `++x`, `x--` e `--x` são alternativas de `expressao_sintatica` e produzem `NO_OP_UNARIA` (ex.: `y = ++a + b--;`). Os comandos isolados `i++;`, `++i;`, `i--;` e `--i;` já existentes em `atribuicao` foram mantidos, com seus nós `NO_INCREMENTO`, `NO_INCREMENTO_ANTES`, `NO_DECREMENTO` e `NO_DECREMENTO_ANTES`. Como as duas formas são ambíguas antes do `;`, o conflito Shift/Reduce é resolvido por precedência: `ID` (menor) e `TOKEN_PTOVIRG` (maior) foram declarados em `%precedence`, de modo que, nesse ponto, o parser escolhe sempre a regra de comando. O incremento do cabeçalho do `for` (`incremento_for`) não foi alterado.
9. **Precedência dos unários:** `++` e `--` têm precedência maior que `!` e `-` (`-a++` é `-(a++)`), e os prefixos usam `%prec UMINUS`.
10. **`NO_LITERAL_BOOL`:** `true` e `false` viram folhas com `valor_int` 1 ou 0. O campo `tipo_dado` não é preenchido nesta etapa.
11. **Condições:** a regra `condicao` não foi alterada. Ela continua produzindo `NO_CONDICAO` sem a expressão como filho (ver `controle_fluxo_ast.md`); a ligação fica para a integração com as condições.

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

### `a < b && c >= d || !flag`: `||` é a raiz, `!` é unário
```text
NO_OP_BINARIA (||)
├── filho[0]: NO_OP_BINARIA (&&)
│   ├── filho[0]: NO_OP_BINARIA (<)
│   │   ├── filho[0]: NO_IDENTIFICADOR (a)
│   │   └── filho[1]: NO_IDENTIFICADOR (b)
│   └── filho[1]: NO_OP_BINARIA (>=)
│       ├── filho[0]: NO_IDENTIFICADOR (c)
│       └── filho[1]: NO_IDENTIFICADOR (d)
└── filho[1]: NO_OP_UNARIA (!)
    └── filho[0]: NO_IDENTIFICADOR (flag)
```

### `-x < 5`: unário tem precedência sobre relacional
```text
NO_OP_BINARIA (<)
├── filho[0]: NO_OP_UNARIA (-)
│   └── filho[0]: NO_IDENTIFICADOR (x)
└── filho[1]: NO_LITERAL_INT (5)
```

### `!(a || b) && true`: parênteses, unário e literal booleano
```text
NO_OP_BINARIA (&&)
├── filho[0]: NO_OP_UNARIA (!)
│   └── filho[0]: NO_OP_BINARIA (||)
│       ├── filho[0]: NO_IDENTIFICADOR (a)
│       └── filho[1]: NO_IDENTIFICADOR (b)
└── filho[1]: NO_LITERAL_BOOL (true)
```

### `y = ++a + b--`: pré-fixado e pós-fixado
```text
NO_ATRIBUICAO
├── filho[0]: NO_IDENTIFICADOR (y)
└── filho[1]: NO_OP_BINARIA (+)
    ├── filho[0]: NO_OP_UNARIA (++)
    │   └── filho[0]: NO_IDENTIFICADOR (a)
    └── filho[1]: NO_OP_UNARIA (--, posfixo)
        └── filho[0]: NO_IDENTIFICADOR (b)
```

---

## 5. Testes

Em `tests/ast/`, executados por `tests/ast/run-tests.sh`. Os positivos comparam a **árvore impressa completa** com o arquivo em `tests/ast/esperado/`.

| Teste | Tipo | Verifica |
|---|---|---|
| `valido_precedencia_aritmetica.java` | positivo | `2 + 3 * 4`, `a + 2 * b`, `10 - 4 - 3`, `8 / 2 % 3`, literais `double` |
| `valido_expressao_com_parenteses.java` | positivo | `(2 + 3) * 4`, `(a + b) / (c - d)`, `2 * (3 + 4) - 5` |
| `invalido_operador_sem_operando.java` | negativo | `a + ;` rejeitado com `Erro sintatico na linha 3` |
| `valido_expressoes_logicas_complexas.java` | positivo | precedência `!`/`-` > relacionais > `&&` > `\|\|`, parênteses e `true` |
| `valido_operadores_incremento.java` | positivo | `++`/`--` pré-fixados e pós-fixados dentro de expressões |
| `invalido_operador_logico_duplicado.java` | negativo | `a && && b;` rejeitado com `Erro sintatico na linha 3` |

```bash
make test-ast
```