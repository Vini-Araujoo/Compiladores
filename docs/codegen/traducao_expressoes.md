# Tradução de Expressões, Literais e Operadores (Issues 10.7 e 10.8)

Especificação e decisões técnicas da geração de código C para literais, identificadores e operadores: aritméticos (`+`, `-`, `*`, `/`, `%`, Issue 10.7), relacionais (`==`, `!=`, `<`, `<=`, `>`, `>=`), lógicos (`&&`, `||`, `!`), unários (`-`, `+`) e de incremento/decremento (`++`, `--`, pré e pós-fixados), além do despachante `emitirExpressao` (Issue 10.8). A implementação está em `src/codegen/codegen_expr.c`.

---

## 1. Visão Geral

O gerador percorre a AST produzida pelo parser e escreve o C equivalente em `FILE *saida`. A função despachante `emitirExpressao` (ponto de entrada público para os demais módulos do gerador) escolhe o emissor pelo campo `tipo_no`:

| `tipo_no` | Função | Observação |
|---|---|---|
| `NO_LITERAL_INT`, `NO_LITERAL_FLOAT`, `NO_LITERAL_DOUBLE`, `NO_LITERAL_CHAR`, `NO_LITERAL_BOOL`, `NO_LITERAL_STRING` | `emitirLiteral` | Seção 2.1 |
| `NO_IDENTIFICADOR` | `emitirIdentificador` | Seção 2.2 |
| `NO_OP_BINARIA` | `emitirOpBinaria`, que classifica o operador e delega a `emitirOpAritmetica`, `emitirOpRelacional` ou `emitirOpLogico` | Seções 2.3 a 2.5 |
| `NO_OP_UNARIA`, `NO_INCREMENTO`, `NO_DECREMENTO`, `NO_INCREMENTO_ANTES`, `NO_DECREMENTO_ANTES` | `emitirOpUnaria` | Seção 2.6 |
| `NO_ACESSO_ARRAY` | `emitirAcessoArray` | Implementada em `codegen_io.c` (outro responsável) |
| `NO_CHAMADA_METODO` | `emitirChamadaMetodo` | Implementada em `codegen_main.c` (outro responsável) |

`emitirExpressao(NULL, ...)` retorna sem emitir nada. Qualquer outro `tipo_no` gera uma mensagem em `stderr` com a linha do fonte.

> `emitirAcessoArray` e `emitirChamadaMetodo` estão declaradas em `codegen.h`, mas pertencem a outros módulos. Enquanto não estiverem implementadas, o link de `codegen_expr.c` precisa de stubs dessas duas funções. São as únicas dependências externas do arquivo.

A hierarquia da expressão já está resolvida na AST (precedência e associatividade vêm das declarações `%left` do Bison). O gerador **não consulta tabela de precedência**: ele só precisa não perder a estrutura da árvore ao escrever texto, o que é feito com parênteses (seção 3, decisão 1).

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

### 2.3 Operadores aritméticos

`NO_OP_BINARIA` tem `filhos[0]` (operando esquerdo), `filhos[1]` (operando direito) e o operador em `valor.operador`. Com `+`, `-`, `*`, `/` ou `%` é emitida como:

```text
(esquerdo op direito)
```

O operando é emitido recursivamente por `emitirExpressao`. Como todo nó recebe parênteses, o texto C reflete exatamente a árvore, e a precedência e a associatividade à esquerda de Java são preservadas por construção:

| Java | C gerado |
|---|---|
| `(a + b) * (c - d)` | `((a + b) * (c - d))` |
| `a - (b - c)` | `(a - (b - c))` |
| `a - b - c` | `((a - b) - c)` |
| `a + b * c` | `(a + (b * c))` |
| `a % b * c` | `((a % b) * c)` |

### 2.4 Operadores relacionais

`==`, `!=`, `<`, `<=`, `>`, `>=` (também `NO_OP_BINARIA`) seguem o mesmo formato `(esquerdo op direito)`. Em C o resultado é um `int` 0 ou 1, que se comporta como o `boolean` de Java ao ser atribuído a uma variável `bool` ou usado como condição.

| Java | C gerado |
|---|---|
| `a < b` | `(a < b)` |
| `a + 1 < b * 2` | `((a + 1) < (b * 2))` |
| `(a + b) > (b - a) * 2` | `((a + b) > ((b - a) * 2))` |
| `(a < b) == (b > a)` | `((a < b) == (b > a))` |

### 2.5 Operadores lógicos

`&&` e `||` (`NO_OP_BINARIA`) são emitidos como `(esquerdo op direito)`. Como a tradução é direta, o **curto-circuito nativo de C** coincide com o de Java: o operando direito só é avaliado quando necessário. A negação `!` (`NO_OP_UNARIA`) é tratada por `emitirOpLogico` e emitida como `(!(operando))`.

| Java | C gerado |
|---|---|
| `a > 0 && b < 10` | `((a > 0) && (b < 10))` |
| `t \|\| f && f` | `(t \|\| (f && f))` |
| `(t \|\| f) && f` | `((t \|\| f) && f)` |
| `!t` | `(!(t))` |
| `!(a > 0 && b < 10)` | `(!(((a > 0) && (b < 10))))` |
| `b != 0 && a / b > 1` | `((b != 0) && ((a / b) > 1))` |

No último exemplo, com `b == 0` a divisão não é executada, nem em Java nem em C.

### 2.6 Operadores unários e incremento/decremento

`emitirOpUnaria` recebe cinco tipos de nó. Todos têm o operando em `filhos[0]`:

| Nó | Operador / origem | Java | C gerado |
|---|---|---|---|
| `NO_OP_UNARIA` | `"-"` | `-x`, `-(a + b)` | `(-x)`, `(-(a + b))` |
| `NO_OP_UNARIA` | `"+"` | `+x` | `(+x)` |
| `NO_OP_UNARIA` | `"!"` | `!x` | `(!(x))` (ver seção 2.5) |
| `NO_OP_UNARIA`, `posfixo = 0` | `"++"` / `"--"` | `++x`, `--x` | `(++x)`, `(--x)` |
| `NO_OP_UNARIA`, `posfixo = 1` | `"++"` / `"--"` | `x++`, `x--` | `(x++)`, `(x--)` |
| `NO_INCREMENTO_ANTES` / `NO_DECREMENTO_ANTES` | comando `++i;` / `--i;` | `++i` / `--i` | `(++i)` / `(--i)` |
| `NO_INCREMENTO` / `NO_DECREMENTO` | comando `i++;` / `i--;` | `i++` / `i--` | `(i++)` / `(i--)` |

Os nós `NO_INCREMENTO*` e `NO_DECREMENTO*` aparecem como **comandos** (`i++;`) e no cabeçalho do `for`; neles `valor.operador` não é usado. O texto emitido não inclui `;`: quem emite o comando acrescenta o ponto e vírgula (`(i++);`).

O valor das expressões coincide com o de Java: o pós-fixado produz o valor antigo e o pré-fixado, o novo.

| Java | C gerado |
|---|---|
| `-a++` | `(-(a++))` |
| `a++ + ++b` | `((a++) + (++b))` |
| `a-- - --b` | `((a--) - (--b))` |
| `-(-a)` | `(-(-a))` |
| `!(a++ > 7)` | `(!(((a++) > 7)))` |

---

## 3. Decisões Técnicas

1. **Parênteses em todo nó (binário, unário e incremento):** é a opção sempre correta e não depende de conhecer a precedência de cada operador. Remover parênteses redundantes é uma otimização possível, mas não foi feita.
2. **`emitirOpBinaria` classifica o operador:** consulta três tabelas (aritméticos, relacionais, lógicos) e delega à função da família. Operador fora das tabelas (`&`, `^`, `<<`...) gera mensagem em `stderr` e nada é emitido. `emitirOpBinaria` continua sendo a função declarada em `codegen.h`; `emitirOpAritmetica`, `emitirOpRelacional` e `emitirOpLogico` são internas (`static`).
3. **Booleanos como `int` 0/1:** o resultado de `==`, `<`, `&&`, `!` etc. em C é `int`. Isso é compatível com `bool` e com condições de `if`/`while`/`for`, e dispensa conversão.
4. **Formato da negação:** `(!(operando))`, com parênteses duplos, como especificado na issue.
5. **Sem tokens colados:** o operando de `-` e `+` que começa com sinal é sempre emitido entre parênteses (`-(-x)`, `-(--x)`, `a - (-b)`). Assim `- -x` nunca vira `--x`, que seria outro operador.
6. **Operando de `++`/`--` via `emitirExpressao`:** serve para identificadores e também para outras formas de l-value, como `v[i]++`, delegado a `emitirAcessoArray`.
7. **`+x` suportado:** o parser atual não gera o `+` unário, mas o gerador já o emite como `(+x)`, conforme a issue.
8. **Strings sem novo escape:** reescapar o conteúdo já cru produziria `\\n` no lugar de `\n`.
9. **Sufixo `LL` em inteiros grandes:** `valor_int` é `long`; valores fora do `int` de 32 bits recebem `LL` para evitar overflow e warning do `gcc`.
10. **Ponto flutuante com a menor precisão exata:** `3.14` sai como `3.14`, e não como `3.1400000000000001` (saída de `%.17g`). Para `float`, o teste de ida e volta é feito com `strtof`, de modo que `0.1f` sai `0.1f` e não `0.100000001f`.
11. **Sem mangling de identificadores:** o nome é copiado como está, conforme a issue.
12. **Nó nulo ou malformado:** `emitirExpressao(NULL, ...)` retorna sem emitir. Operando ausente, número de filhos incorreto ou operador desconhecido geram mensagem em `stderr` com a linha do fonte e não escrevem nada, em vez de derrubar o gerador.

---

## 4. Limitações Conhecidas

Divergências entre Java e C que dependem do tipo dos operandos e ainda não são tratadas. `inferencia.c` (Issue 9.3) preenche `tipo_dado` nas expressões, mas resolve identificadores com `buscarTipoSimboloMock` (a macro `TABELA_INTEGRADA` não é definida em lugar algum), então ainda não é uma fonte confiável para o gerador:

- `x / 0` e `x % 0` com inteiros: Java lança `ArithmeticException`; em C é comportamento indefinido. O divisor **literal** `0` já é rejeitado pela análise semântica (Issue 9.5); divisor que vale zero em tempo de execução (variável, expressão) continua indefinido.
- `INT_MIN / -1`: Java retorna `INT_MIN`; em C é indefinido.
- `%` entre `double`: válido em Java, inválido em C (exigiria `fmod`).
- Overflow de inteiro com sinal: Java dá wrap-around; em C é indefinido (considerar `-fwrapv`). Vale também para `x++` com `x == INT_MAX` e para `-x` com `x == INT_MIN`.
- Identificadores que são palavras reservadas de C, mas não de Java (`auto`, `register`, `signed`, `unsigned`, `typedef`, `sizeof`, `extern`, `union`, `inline`), são emitidos sem alteração e quebram o C gerado.

Limitações herdadas do front-end:

- O lexer não reconhece literais `char` (`'a'`), sufixos `f`, `L` e `d`, expoente (`1e3`), hexadecimal, binário e `_` em números. Por isso o parser ainda não gera `NO_LITERAL_FLOAT` nem `NO_LITERAL_CHAR`; o gerador os suporta, mas só foram testados com nós montados à mão.
- O parser não gera o `+` unário (`+x`); só `!`, `-`, `++` e `--`.
- O analisador de tipos (`inferencia.c`, usado pelo binário `build/semantic`) ainda não reconhece `NO_LITERAL_STRING`, tipa `==` e `!=` como numéricos e só trata como `boolean` o identificador `flag` (mock de símbolos). Os testes foram escritos para passar por esse binário: usam `flag` como único `boolean` em operações lógicas, evitam `==`/`!=` como operando de `&&`/`||` e não comparam dois booleanos com `==`. A exceção é `valido_expr_todos_literais.java`, que declara uma `String`: passa pelo parser, mas é rejeitado pelo binário `semantic` enquanto o literal de string não for tipado.
- `-2147483648`: o lexer lê `2147483648` e o `-` vira unário, resultando em `(-2147483648LL)` (`long long`, não `int`).

Sobre os avisos do `gcc`: o C gerado reproduz o fonte. Um fonte Java válido como `d == d` ou `x != x` gera `-Wtautological-compare` ao compilar o C com `-Wall`, e isso não é defeito do gerador.

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

### `a < b && c >= d || !flag`: `||` é a raiz, `!` é unário
```text
NO_OP_BINARIA (||)                    (((a < b) && (c >= d)) || (!(flag)))
├── NO_OP_BINARIA (&&)
│   ├── NO_OP_BINARIA (<)
│   │   ├── NO_IDENTIFICADOR (a)
│   │   └── NO_IDENTIFICADOR (b)
│   └── NO_OP_BINARIA (>=)
│       ├── NO_IDENTIFICADOR (c)
│       └── NO_IDENTIFICADOR (d)
└── NO_OP_UNARIA (!)
    └── NO_IDENTIFICADOR (flag)
```

### `y = ++a + b--`: pré-fixado e pós-fixado
```text
NO_OP_BINARIA (+)                     ((++a) + (b--))
├── NO_OP_UNARIA (++)
│   └── NO_IDENTIFICADOR (a)
└── NO_OP_UNARIA (--, posfixo)
    └── NO_IDENTIFICADOR (b)
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

Em `tests/codegen/`, executados por `tests/codegen/run-tests.sh`:

```bash
make test-codegen
```

O alvo compila `tests/codegen/teste_codegen_expressoes.c` (parser real + `codegen_expr.c` + `codegen_main.c`) em `build/teste_codegen_expressoes` e chama o script. O programa de teste lê um `.java`, emite com `emitirExpressao` a tradução de cada declaração com inicializador e de cada comando `i++;`/`--i;`, na ordem do fonte, e imprime o valor de todas as variáveis ao final. A emissão de declarações e de impressão é andaime do teste, pois pertence a outras issues.

Cada teste passa por três etapas e só conta como aprovado se passar por todas:

| Etapa | Verifica |
|---|---|
| Tradução | O C gerado, a partir de `int main`, é idêntico a `esperado/<nome>.c.txt`. O cabeçalho (`emitirCabecalho`) fica fora da comparação. |
| Compilação | O C gerado compila com `gcc -Wall -Wextra -Werror`. |
| Execução | Se existir `esperado/<nome>.saida.txt`, a saída do programa C é idêntica a ele. |

Testes:

| Teste | Issue | Tipo | Verifica |
|---|---|---|---|
| `valido_expr_aritmetica_simples.java` | 10.7 | positivo | `+`, `-`, `*`, `/` e `%` entre variáveis `int` |
| `valido_expr_aritmetica_composta.java` | 10.7 | positivo | `(a + b) * (c - d)`, `a - (b - c)`, `a - b - c`, `a / (b * d)`, `a % b * c`, `a * (b % c)`, `a + b * c - d / 2` |
| `valido_expr_todos_literais.java` | 10.7 | positivo | literais `int`, `double`, `boolean` e `String` (com aspas escapadas e `\n`) |
| `valido_expr_relacionais.java` | 10.8 | positivo | os seis relacionais, `int` contra `double`, relacionais com aritmética e com parênteses |
| `valido_expr_logicos_curto_circuito.java` | 10.8 | positivo | `&&`, `\|\|`, `!`, precedência `&&` sobre `\|\|`, parênteses, e curto-circuito evitando a divisão por zero (`b > 0 && a / b > 1`, `b <= 0 \|\| a / b > 1`, com `b = 0`) |
| `valido_expr_unarios_pre_pos.java` | 10.8 | positivo | `-x`, `-(a + b)`, `-(-a)`, `x++`, `++x`, `x--`, `--x` em inicializadores, `-a++`, `a++ + ++b`, `a-- - --b`, `!(a++ > 7)` e os comandos `a++;` e `--b;` |

Os arquivos `esperado/<nome>.saida.txt` foram obtidos **executando o mesmo programa em Java** (com um `println` de cada variável), e não a partir do C gerado, para que a comparação seja independente do gerador. `valido_expr_todos_literais.java` não tem `.saida.txt`: o Java e o C formatam `double` de forma diferente (`2.0` e `2`), então só a tradução e a compilação são verificadas.

Para regenerar um `esperado/<nome>.c.txt` depois de uma mudança intencional no gerador:

```bash
build/teste_codegen_expressoes < tests/codegen/<nome>.java | sed -n '/^int main(void) {/,$p' > tests/codegen/esperado/<nome>.c.txt
```

> `emitirAcessoArray` e `emitirChamadaMetodo` ainda não existem; o programa de teste define versões fracas (`weak`) delas que apenas avisam em `stderr`. Quando os outros módulos as implementarem, a definição forte prevalece no link, sem alterar o teste.

> O programa de teste imprime todas as variáveis, então o C gerado não tem `unused variable`. No pipeline real, quando o `codegen_decl.c` traduzir as declarações de um programa que não usa suas variáveis, o `gcc -Wall` emitirá esse aviso. Isso vem do programa Java, não das expressões.