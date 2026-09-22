# Documentação Sintática: Expressões e Precedência de Operadores

- **Issue:** Issue 2
- **Responsável:** Pedro Augusto Ribeiro Faitarone Bessa
- **Branch:** `feat/issue-2-expressoes`

---

## 1. Descrição Geral

Este documento especifica a gramática sintática de expressões implementada em `parser/parser.y`, cobrindo:

- Expressões aritméticas binárias (`+`, `-`, `*`, `/`);
- Expressões relacionais (`<`, `>`, `<=`, `>=`) e de igualdade (`==`, `!=`);
- Operadores unários de negação lógica (`!`) e de sinal (`-a`);
- Agrupamento por parênteses `( expressao )`;
- Fatores primários: identificadores (`ID`) e literais numéricos (`NUMERO`).

A gramática é declarada no Bison com as diretivas `%left` e `%precedence`, o que resolve automaticamente as ambiguidades naturais de uma gramática de expressões (que, sem anotação de precedência, geraria conflitos Shift/Reduce).

Como ponto de entrada temporário para testar a regra `expressao` de forma isolada (sem depender de declarações, comandos ou classes, que são objetos de outras issues), foi definida a regra inicial:

```
programa:
      lista_expressoes
    ;

lista_expressoes:
      %empty
    | lista_expressoes expressao TOKEN_PTOVIRG
    ;
```

Isso permite validar arquivos com sequências de expressões terminadas em `;`
(ex.: `(a + 2) * b >= c == !flag;`) diretamente com `make test ISSUE=2`.

---

## 2. Tabela de Precedência e Associatividade

Do menor para o maior nível de precedência:

| Nível (crescente)| Operadores | Associatividade | Categoria |
|---|---|---|---|
| 1 (mais baixo)      | `==` `!=`                         | Esquerda (`%left`)   | Igualdade                       |
| 2                   | `<` `>` `<=` `>=`                 | Esquerda (`%left`)   | Relacional                      |
| 3                   | `+` `-` (binários)                | Esquerda (`%left`)   | Aritmética (soma/subtração)     |
| 4                   | `*` `/`                           | Esquerda (`%left`)   | Aritmética (multiplicação/divisão) |
| 5 (mais alto)       | `!` (negação lógica), `-` (unário)| N/A (`%precedence`)  | Unários                         |

Correspondência com a declaração no Bison (`parser/parser.y`):

```
%left   TOKEN_IGUAL TOKEN_DIFERENTE
%left   TOKEN_MENOR TOKEN_MAIOR TOKEN_MENOR_IGUAL TOKEN_MAIOR_IGUAL
%left   TOKEN_MAIS TOKEN_MENOS
%left   TOKEN_VEZES TOKEN_DIV
%precedence TOKEN_NEGACAO UMINUS
```

Como no Bison a precedência das diretivas aumenta de cima para baixo, a ordem acima garante que `*`/`/` amarrem mais forte que `+`/`-`, que por sua vez amarram mais forte que os relacionais, que amarram mais forte que a igualdade, exatamente igual a ordem convencional de avaliação matemática e lógica.

---

## 3. Resolução de Ambiguidades

### 3.1 Operadores binários (aritméticos, relacionais e de igualdade)

A gramática de expressões é naturalmente ambígua quando escrita de forma recursiva simples (`expressao : expressao OP expressao | ...`), pois uma entrada como `2 + 3 * 4` pode ser derivada de mais de uma forma (agrupando `(2 + 3) * 4` ou `2 + (3 * 4)`), e o Bison, ao encontrar o próximo operador, não sabe se deve reduzir a expressão já lida (`reduce`) ou empilhar o novo operador (`shift`), e isso gera um conflito Shift/Reduce.

Isso é resolvido declarando a precedência relativa de cada token operador (via `%left`) na ordem da seção 2. Ao decidir entre shift e reduce, o Bison compara a precedência do token à frente (o operador que seria "empurrado") com a precedência da regra que seria reduzida (dada pelo seu terminal mais à direita), o de maior precedência vence. Como todos os operadores binários são associativos à esquerda (`%left`), em caso de empate de precedência (ex.: `10 - 3 - 2`) o parser sempre reduz primeiro, produzindo a associação à esquerda `(10 - 3) - 2`, que é o comportamento aritmético convencional da matemática.

Isso também resolve a precedência entre categorias diferentes: como `TOKEN_VEZES`/`TOKEN_DIV` foram declarados depois de `TOKEN_MAIS`/`TOKEN_MENOS`, têm precedência maior, então `2 + 3 * 4` é sempre lido como `2 + (3 * 4)`; da mesma forma, os operadores relacionais têm precedência maior que os de igualdade, então `a + 2 * b >= c == !flag` é lido como `((a + (2 * b)) >= c) == (!flag)`.

### 3.2 Menos unário (`-a`) vs. subtração binária (`a - b`)

### 3.2 Menos unário (`-a`) vs. subtração binária (`a - b`)

O token `TOKEN_MENOS` é usado em duas regras: subtração binária (`expressao TOKEN_MENOS expressao`) e menos unário (`TOKEN_MENOS expressao`). Isso gera ambiguidade: ao ler `-a - b`, ao chegar no segundo `-` o parser precisa decidir se o operando do menos unário já terminou em `a` (produzindo `(-a) - b`) ou se deve se estender para incluir o restante da expressão antes de aplicar a negação (produzindo `-(a - b)`), e essa é a decisão comum de shift/reduce.

Por padrão, o Bison atribui a uma regra a precedência do seu terminal mais à direita, nesse caso, a regra do menos unário herdaria a precedência do próprio `TOKEN_MENOS` (nível 3, mesmo nível de `+`/`-` binários). Contra operadores desse mesmo nível (`-a - b`, `-a + b`) isso até resolve corretamente por coincidência, pois o empate de precedência é desfeito pela associatividade à esquerda (reduz primeiro), dando `(-a) - b` e `(-a) + b`. Apesar disso, um problema aparece com operadores de **precedência maior**, como `*`/`/`: em `-a * b`, como `*` (nível 4) tem precedência maior que a regra do menos unário (nível 3 por padrão), o Bison prefere shift a reduce e estende o operando do menos unário para `a * b` antes de negar, produzindo `-(a * b)` em vez do `(-a) * b` esperado, o menos unário deixa de ter "prioridade máxima sobre operadores binários" como pede a especificação. Para resolver isso de forma explícita e independente de coincidências de associatividade, a regra do menos unário usa a diretiva `%prec UMINUS`:

```
expressao:
      ...
    | TOKEN_MENOS expressao %prec UMINUS
    ;
```

`UMINUS` é um pseudo-token (não é retornado pelo lexer, existe apenas na gramática) declarado no nível de maior precedência (`%precedence TOKEN_NEGACAO UMINUS`). Ao usar `%prec UMINUS`, a regra do menos unário passa a ter a precedência mais alta da gramática, maior até que `*`/`/`, então em qualquer confronto shift/reduce contra um operador binário o Bison sempre reduz primeiro, fazendo o menos unário se ligar apenas ao operando imediatamente à sua direita. Isso garante `-x + 10` como `(-x) + 10` e, principalmente, corrige o caso de `*`/`/`: `-a * b` passa a ser lido como `(-a) * b`, e não mais `-(a * b)`, exatamente como especificado ("prioridade máxima sobre operadores binários").

### 3.3 Negação lógica (`!`)

A regra `TOKEN_NEGACAO expressao` não precisa de `%prec` explícito, pois `TOKEN_NEGACAO` é o único terminal da regra e não é usado em nenhuma outra posição da gramática, sua precedência (a mais alta, junto com `UMINUS`) já é a precedência padrão da própria regra. Isso garante, por exemplo, que `!(x > 5)` só é válido por causa dos parênteses (sem eles, `!x > 5` seria lido como `(!x) > 5`, pois `!` amarra mais forte que `>`).

### 3.4 Agrupamento por parênteses

A regra `TOKEN_ABRE_PAR expressao TOKEN_FECHA_PAR` não introduz ambiguidade: ao ver `(`, o parser sempre desloca (shift) e espera uma `expressao` completa seguida de `)`, sem nenhuma alternativa para essa combinação de tokens. Isso permite forçar qualquer ordem de avaliação, como em `(2 + 3) * 4` ou `(a + b) / (c - d)`.

---

## 4. Gramática Formal (BNF)

```bnf
<programa>          ::= <lista_expressoes>

<lista_expressoes>  ::= /* vazio */
                       | <lista_expressoes> <expressao> ";"

<expressao>         ::= <expressao> "+" <expressao>
                       | <expressao> "-" <expressao>
                       | <expressao> "*" <expressao>
                       | <expressao> "/" <expressao>
                       | <expressao> "<" <expressao>
                       | <expressao> ">" <expressao>
                       | <expressao> "<=" <expressao>
                       | <expressao> ">=" <expressao>
                       | <expressao> "==" <expressao>
                       | <expressao> "!=" <expressao>
                       | "!" <expressao>
                       | "-" <expressao>
                       | "(" <expressao> ")"
                       | ID
                       | NUMERO
```

> Observação: esta BNF é ambígua por natureza (não expressa precedência nem associatividade em sua forma pura). A desambiguação efetiva, conforme descrito na seção 3, é feita através das diretivas de precedência do Bison (`%left`, `%precedence`, `%prec`).

---

## 5. Contrato de Interface

- **Não-terminal produzido:** `expressao`, a ser consumido pela Issue 1 (declarações/atribuições), pela Issue 3 (condições de `if`/`while`/`for`) e pela Issue 5 (índices de array e chamadas de método).
- **Não-terminais consumidos:** nenhum, a regra `expressao` trabalha diretamente sobre os tokens léxicos (`ID`, `NUMERO` e operadores) emitidos por `lexer/lexer.l`.

---

## 6. Como Executar os Testes

```bash
make test ISSUE=2
```

O script `tests/parser/run-tests.sh`:

- compila o parser (`make parser`);
- executa cada arquivo `tests/parser/issue_2/valido_*.java` e falha se o parser retornar código de saída diferente de `0`;
- executa cada arquivo `tests/parser/issue_2/invalido_*.java` e falha se o parser **não** retornar um código de saída diferente de `0`  (ou seja, se aceitar uma entrada que deveria ser rejeitada);
- imprime um resumo com o total de casos e o total de falhas, retornando código de saída `1` caso alguma falha tenha ocorrido.

## Casos de Teste Validados

- [x] `tests/parser/issue_2/valido_aritmetica_basica.java`
- [x] `tests/parser/issue_2/valido_precedencia_e_parenteses.java`
- [x] `tests/parser/issue_2/valido_relacionais_e_igualdade.java`
- [x] `tests/parser/issue_2/valido_unarios_e_complexas.java`
- [x] `tests/parser/issue_2/invalido_operador_duplicado.java`
- [x] `tests/parser/issue_2/invalido_parenteses_abertos.java`
- [x] `tests/parser/issue_2/invalido_operador_sem_operando.java`
- [x] `tests/parser/issue_2/invalido_expressao_vazia.java`
