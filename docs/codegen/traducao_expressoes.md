# Tradução de Expressões Aritméticas e Literais (Issue 10.7)

Especificação e decisões técnicas da geração de código C para literais, identificadores e expressões binárias (`+`, `-`, `*`, `/`, `%`). A implementação está em `src/codegen/codegen_expr.c`.

---

## 1. Visão Geral

O gerador percorre a AST produzida pelo parser e escreve o C equivalente em `FILE *saida`. A função despachante `emitirExpressao` escolhe o emissor pelo campo `tipo_no`:

| `tipo_no` | Função | Observação |
|---|---|---|
| `NO_LITERAL_INT`, `NO_LITERAL_FLOAT`, `NO_LITERAL_DOUBLE`, `NO_LITERAL_CHAR`, `NO_LITERAL_BOOL`, `NO_LITERAL_STRING` | `emitirLiteral` | Seção 2.1 |
| `NO_IDENTIFICADOR` | `emitirIdentificador` | Seção 2.2 |
| `NO_OP_BINARIA` | `emitirOpBinaria` | Seção 2.3 |

> `NO_OP_UNARIA` (`!`, `-`, `++`, `--`) não faz parte desta issue: o `case` no despachante e `emitirOpUnaria` (já declarada em `codegen.h`) serão adicionados pela Issue 10.8. Até lá, um nó unário cai no `default` e gera uma mensagem em `stderr`.

A hierarquia da expressão já está resolvida na AST (precedência e associatividade vêm das declarações `%left` do Bison). O gerador **não consulta tabela de precedência**: ele só precisa não perder a estrutura da árvore ao escrever texto, o que é feito com parênteses (seção 2.3).

---

## 2. Mapeamento

### 2.1 Literais

| Nó | Campo | Saída em C | Observações |
|---|---|---|---|
| `NO_LITERAL_INT` | `valor.valor_int` (`long`) | `42` | Acima de `2147483647` recebe o sufixo `LL`. Valor negativo sairia como `(-5LL)`, mas o parser nunca gera literal negativo: `-5` é `NO_OP_UNARIA` sobre `5`. |
| `NO_LITERAL_FLOAT` | `valor.valor_double` | `1.5f` | Mesma regra do `double`, com precisão máxima de 9 dígitos e sufixo `f` (sem ele, `gcc` converteria `double` para `float` implicitamente). |
| `NO_LITERAL_DOUBLE` | `valor.valor_double` | `3.14159` | Usa a menor precisão (`%.{1..17}g`) que reproduz o mesmo `double` ao voltar por `strtod`. Se o texto não tiver `.` nem expoente, acrescenta `.0` (`2` vira `2.0`). |
| `NO_LITERAL_CHAR` | `valor.valor_int` (código do caractere) | `'a'` | Escapa `\n`, `\t`, `\r`, `\\`, `\'` e `\0`; demais não imprimíveis saem em octal (`'\310'`). O `char` de Java é UTF-16: códigos acima de 255 saem como número (`8364`). |
| `NO_LITERAL_BOOL` | `valor.valor_int` (1 ou 0) | `true` / `false` | Exige `<stdbool.h>`, já emitido por `emitirCabecalho`. |
| `NO_LITERAL_STRING` | `valor.valor_str` | `"texto"` | O lexer guarda o conteúdo entre as aspas **sem decodificar** os escapes. `\n`, `\t`, `\"` e `\\` têm o mesmo significado em C, então o texto só é cercado por aspas, sem novo escape. |

> O contrato de armazenamento de `FLOAT` (em `valor_double`) e `CHAR` (em `valor_int`) segue o impressor da AST (`src/ast/ast.c`). O lexer e o parser atuais ainda não produzem esses dois nós (ver seção 4); o gerador já os trata e foi exercitado com nós montados manualmente.

### 2.2 Identificadores

`NO_IDENTIFICADOR` é emitido com o nome guardado em `valor.valor_str`, sem alteração. Nó sem nome gera mensagem em `stderr` e nada é escrito.

> A issue cita `NO_ID`; em `src/tipos.h` ele é um alias (`#define NO_ID NO_IDENTIFICADOR`), então o `case NO_IDENTIFICADOR` já cobre os dois nomes.

### 2.3 Operações binárias

`NO_OP_BINARIA` tem `filhos[0]` (operando esquerdo), `filhos[1]` (operando direito) e o operador em `valor.operador`. É emitida sempre como:

```text
(esquerdo op direito)
```

O operando é emitido recursivamente por `emitirExpressao`. Como todo nó binário recebe parênteses, o texto C reflete exatamente a árvore, e a precedência e a associatividade à esquerda de Java são preservadas por construção:

| Java | C gerado |
|---|---|
| `(a + b) * (c - d)` | `((a + b) * (c - d))` |
| `a - (b - c)` | `(a - (b - c))` |
| `a - b - c` | `((a - b) - c)` |
| `a + b * c` | `(a + (b * c))` |
| `a % b * c` | `((a % b) * c)` |

> A issue cita `emitirOpAritmetica`; o protótipo declarado em `codegen.h` é `emitirOpBinaria`, que é o nome implementado.

---

## 3. Decisões Técnicas

1. **Parênteses em todo nó binário:** é a opção sempre correta e não depende de conhecer a precedência de cada operador. Remover parênteses redundantes é uma otimização possível, mas não foi feita.
2. **Uma única função para todos os binários:** o parser usa `NO_OP_BINARIA` também para relacionais (`==`, `!=`, `<`, `>`, `<=`, `>=`) e lógicos (`&&`, `||`), e a grafia desses operadores em C é idêntica à de Java. `emitirOpBinaria` os cobre sem código extra.
3. **Strings sem novo escape:** reescapar o conteúdo já cru produziria `\\n` no lugar de `\n`.
4. **Sufixo `LL` em inteiros grandes:** `valor_int` é `long`; valores fora do `int` de 32 bits recebem `LL` para evitar overflow e warning do `gcc`.
5. **Ponto flutuante com a menor precisão exata:** `3.14` sai como `3.14`, e não como `3.1400000000000001` (saída de `%.17g`). Para `float`, o teste de ida e volta é feito com `strtof`, de modo que `0.1f` sai `0.1f` e não `0.100000001f`.
6. **Sem mangling de identificadores:** o nome é copiado como está, conforme a issue.
7. **Operando nulo ou nó malformado:** gera mensagem em `stderr` com a linha do fonte e não escreve nada, em vez de derrubar o gerador.
8. **Unários ficam para a Issue 10.8:** o despachante desta issue não referencia `emitirOpUnaria`, então `codegen_expr.c` compila e liga sozinho. Os operadores relacionais e lógicos binários (`==`, `<`, `&&`, `||`...) não dependem da 10.8, pois já passam por `emitirOpBinaria` (decisão 2).

---

## 4. Limitações Conhecidas

Divergências entre Java e C que dependem do tipo dos operandos e ainda não são tratadas. `inferencia.c` (Issue 9.3) preenche `tipo_dado` nas expressões, mas resolve identificadores com `buscarTipoSimboloMock` (a macro `TABELA_INTEGRADA` não é definida em lugar algum), então ainda não é uma fonte confiável para o gerador:

- `x / 0` e `x % 0` com inteiros: Java lança `ArithmeticException`; em C é comportamento indefinido. O divisor **literal** `0` já é rejeitado pela análise semântica (Issue 9.5); divisor que vale zero em tempo de execução (variável, expressão) continua indefinido.
- `INT_MIN / -1`: Java retorna `INT_MIN`; em C é indefinido.
- `%` entre `double`: válido em Java, inválido em C (exigiria `fmod`).
- Overflow de inteiro com sinal: Java dá wrap-around; em C é indefinido (considerar `-fwrapv`).
- Ordem de avaliação dos operandos: Java avalia da esquerda para a direita; C não especifica (relevante quando houver chamadas de método e `++` na mesma expressão).
- Identificadores que são palavras reservadas de C, mas não de Java (`auto`, `register`, `signed`, `unsigned`, `typedef`, `sizeof`, `extern`, `union`, `inline`), são emitidos sem alteração e quebram o C gerado.

Limitações herdadas do front-end:

- O lexer não reconhece literais `char` (`'a'`), sufixos `f`, `L` e `d`, expoente (`1e3`), hexadecimal, binário e `_` em números. Por isso o parser ainda não gera `NO_LITERAL_FLOAT` nem `NO_LITERAL_CHAR`; o gerador os suporta, mas só foram testados com nós montados à mão.
- `-2147483648`: o lexer lê `2147483648` e o `-` vira unário, resultando em `-2147483648LL` (`long long`, não `int`).

---

## 5. Exemplos de Tradução

### `(a + b) * (c - d)`
```text
NO_OP_BINARIA (*)                     ((a + b) * (c - d))
├── NO_OP_BINARIA (+)
│   ├── NO_IDENTIFICADOR (a)
│   └── NO_IDENTIFICADOR (b)
└── NO_OP_BINARIA (-)
    ├── NO_IDENTIFICADOR (c)
    └── NO_IDENTIFICADOR (d)
```

### `a - (b - c)` e `a - b - c`: a associatividade aparece nos parênteses
```text
a - (b - c)  ->  (a - (b - c))
a - b - c    ->  ((a - b) - c)
```

### Literais
```text
42                ->  42
2147483648        ->  2147483648LL
3.14159           ->  3.14159
2.0               ->  2.0
1.5f   (FLOAT)    ->  1.5f
'a'    (CHAR)     ->  'a'
'\n'   (CHAR)     ->  '\n'
true              ->  true
"texto \"x\"\n"   ->  "texto \"x\"\n"
```

---

## 6. Testes

Em `tests/codegen/`. Cada arquivo deve gerar C que compila sem warnings com `gcc -Wall -Wextra -Werror`.

| Teste | Tipo | Verifica |
|---|---|---|
| `valido_expr_aritmetica_simples.java` | positivo | `+`, `-`, `*`, `/` e `%` entre variáveis `int` |
| `valido_expr_aritmetica_composta.java` | positivo | `(a + b) * (c - d)`, `a - (b - c)`, `a - b - c`, `a / (b * d)`, `a % b * c`, `a * (b % c)`, `a + b * c - d / 2` |
| `valido_expr_todos_literais.java` | positivo | literais `int`, `double`, `boolean` e `String` (com aspas escapadas e `\n`) |

> Os três programas declaram variáveis que nunca são usadas. Quando o `codegen_decl.c` traduzir as declarações, o `gcc -Wall` emitirá `unused variable` para elas; isso vem do programa de teste, não das expressões. Para o critério "sem warnings", use as variáveis nos testes (por exemplo, com `System.out.println`) ou compile com `-Wno-unused-variable`.

Para conferir o resultado numérico, compile o programa Java com `javac` e o C gerado com `gcc`, e compare as saídas. Use operandos inteiros positivos e pequenos, para não esbarrar nas divergências da seção 4.