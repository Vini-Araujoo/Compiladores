# Tradução de Tipos Java → C, Declarações, Atribuições e Blocos (Issues 10.9 e 10.10)

Este documento descreve a especificação técnica e a implementação do mapeamento de tipos primitivos, booleanos e strings do Java para a linguagem C, da geração de código C para declarações de variáveis (simples, inicializadas, múltiplas, constantes `final` e `Scanner`), bem como da tradução de atribuições (simples, compostas e em arrays) e blocos de escopo aninhados `{ ... }`.

O código deste módulo reside exclusivamente em `src/codegen/codegen_decl.c` e inclui apenas `"codegen.h"`, preservando o isolamento para desenvolvimento paralelo.

---

## 1. Mapeamento de Tipos (`traduzirTipoC`)

A função `const char *traduzirTipoC(TipoDado tipo)` converte o enum `TipoDado` (definido em `src/tipos.h`) para a representação textual correspondente em C:

| `TipoDado` (AST) | Tipo Java | Tipo C gerado | Cabeçalho / Observação |
| --- | --- | --- | --- |
| `TIPO_BYTE` | `byte` | `char` | Inteiro de 1 byte |
| `TIPO_SHORT` | `short` | `short` | Inteiro curto |
| `TIPO_INT` | `int` | `int` | Inteiro padrão |
| `TIPO_LONG` | `long` | `long long` | Inteiro longo de 64 bits |
| `TIPO_FLOAT` | `float` | `float` | Ponto flutuante de precisão simples |
| `TIPO_DOUBLE` | `double` | `double` | Ponto flutuante de precisão dupla |
| `TIPO_CHAR` | `char` | `char` | Caractere |
| `TIPO_BOOLEAN` | `boolean` | `bool` | Requer `#include <stdbool.h>` (emitido por `emitirCabecalho`) |
| `TIPO_STRING` | `String` | `char *` | Ponteiro para cadeia de caracteres (`char *s` ou `const char *s`) |
| `TIPO_VOID` | `void` | `void` | Retorno vazio |
| `TIPO_SCANNER` | `Scanner` | `NULL` | Dispensável em C (`<stdio.h>` já disponibiliza `stdin`) |
| `TIPO_ARRAY` / `TIPO_INDEFINIDO` | — | `NULL` | Sem tradução primitiva direta neste módulo |

---

## 2. Tradução de Declarações (`emitirDeclaracaoVariavel`)

A função `void emitirDeclaracaoVariavel(NoAST *no, FILE *saida, int nivel)` traduz nós `NO_DECL_VARIAVEL` (alias `NO_VAR_DECL`) para instruções C devidamente indentadas conforme `nivel` (usando `emitirIndentacao` / `emitirLinha` de `codegen_main.c`) e terminadas em `;\n`.

### 2.1 Declarações Simples e Inicializadas

| Código Java | Código C gerado |
| --- | --- |
| `byte b;` | `char b;` |
| `short s;` | `short s;` |
| `int x = 10;` | `int x = 10;` |
| `long l = 1000;` | `long long l = 1000;` |
| `float f = 10;` | `float f = 10;` |
| `double d = 3.14;` | `double d = 3.14;` |
| `char c;` | `char c;` |
| `boolean ativo = true;` | `bool ativo = true;` |
| `String texto = "Ola";` | `char *texto = "Ola";` |

Regras aplicadas:
- Para tipos terminados em `*` (`char *`), o nome da variável é emitido diretamente após o asterisco (`char *texto`), evitando espaço redundante.
- Quando há expressão inicializadora comum, emite-se ` = ` seguido da delegação para `emitirExpressao(inicializador, saida)` (módulo de expressões em `codegen_expr.c`).
- Suporta ambos os formatos de representação de `NO_DECL_VARIAVEL`:
  1. **Formato padrão do parser**: nome em `no->valor.valor_str` e inicializador opcional em `no->filhos[0]`.
  2. **Formato alternativo com identificador filho**: `no->valor.valor_str == NULL`, `no->filhos[0]` como `NO_IDENTIFICADOR` e inicializador opcional em `no->filhos[1]`.

### 2.2 Constantes `final` → `const`

A flag de imutabilidade é verificada em duas fontes para cobrir tanto nós recém-construídos pelo parser quanto nós anotados pela análise semântica:
- `no->mods.eh_final`
- `no->simbolo && (no->simbolo->eh_final || no->simbolo->mods.eh_final)`

Quando verdadeira, o qualificador `const ` é prefixado antes do tipo C, garantindo que o compilador C (`gcc`) impeça qualquer modificação subsequente no binário gerado:

| Código Java | Código C gerado |
| --- | --- |
| `final int MAX = 100;` | `const int MAX = 100;` |
| `final double PI = 3.14159;` | `const double PI = 3.14159;` |
| `final boolean HABILITADO = true;` | `const bool HABILITADO = true;` |
| `final String TITULO = "Compilador";` | `const char *TITULO = "Compilador";` |

### 2.3 Declarações Múltiplas

O gerador suporta tanto declarações já desmembradas pelo parser (`adicionar_declaracoes_a_pai`) quanto **contêineres `NO_DECL_VARIAVEL`** (`valor_str == NULL` contendo filhos `NO_DECL_VARIAVEL`), como em declarações encadeadas ou cabeçalhos de laços:

```java
int a = 1, b, c = 3;
final int K1 = 10, K2 = 20;
```

Código C gerado:

```c
int a = 1;
int b;
int c = 3;
const int K1 = 10;
const int K2 = 20;
```

O contêiner propaga automaticamente `tipo_dado` e o qualificador `final` do nó pai para cada declaração filha sem mutar a AST original.

### 2.4 Casos Especiais de `Scanner`

#### Declaração de instância `Scanner`
Como `<stdio.h>` já oferece `stdin` em C, variáveis do tipo `TIPO_SCANNER` não declaram `struct` nem ponteiro — emitem apenas um comentário descritivo com o recuo correspondente:

```java
Scanner sc = new Scanner(System.in);
```

```c
/* Scanner sc */
```

#### Inicialização com leitura de `Scanner`
Quando o inicializador de uma variável é um nó `NO_LEITURA_SCANNER` (diretamente ou encapsulado em `NO_EXPRESSAO`), a declaração e a leitura são separadas em duas instruções:
1. Emite-se a declaração da variável sem inicializador (`int x;\n`). Para `TIPO_STRING` lida via `Scanner` (`next()` / `nextLine()`), emite-se um buffer estático `char nome[256];\n` para que o destino do `scanf` possua memória válida alocada.
2. Delega-se a emissão do `scanf` à função `emitirScannerLeitura(leitura, saida, nivel)` do módulo de I/O (`src/codegen/codegen_io.c`).

```java
int idade = sc.nextInt();
double altura = sc.nextDouble();
String nome = sc.nextLine();
```

Código C esperado após integração com `codegen_io.c`:

```c
int idade;
scanf("%d", &idade);
double altura;
scanf("%lf", &altura);
char nome[256];
scanf(" %[^\n]", nome);
```

---

## 3. Tradução de Atribuições (`emitirAtribuicao` — Issue 10.10)

A função `void emitirAtribuicao(NoAST *no, FILE *saida, int nivel)` traduz nós `NO_ATRIBUICAO` (e comandos unários de incremento/decremento produzidos pela regra sintática `atribuicao`) para instruções C com indentação correspondente a `nivel` e terminadas em `;\n`.

### 3.1 Operadores Suportados e Precedência

O operador é obtido de `no->valor.operador` (com fallback seguro para `"="` caso esteja vazio) e emitido com espaços ao redor (`<alvo> <op> <expressao>;`):

| Operador | Descrição | Exemplo Java | Código C gerado |
| --- | --- | --- | --- |
| `=` | Atribuição simples | `resultado = (a + b) * 2;` | `resultado = ((a + b) * 2);` |
| `+=` | Adição e atribuição | `total += passo * 2;` | `total += (passo * 2);` |
| `-=` | Subtração e atribuição | `total -= passo + 5;` | `total -= (passo + 5);` |
| `*=` | Multiplicação e atribuição | `x *= a + 1;` | `x *= (a + 1);` |
| `/=` | Divisão e atribuição | `x /= 2;` | `x /= 2;` |
| `%=` | Módulo e atribuição | `x %= 4;` | `x %= 4;` |

> **Observação de gramática:** `emitirAtribuicao` já suporta os 6 operadores (`=`, `+=`, `-=`, `*=`, `/=`, `%=`) na camada de geração de código; no lexer/parser atual (`lexer/lexer.l` e `parser/parser.y`), os tokens definidos para atribuição são `=`, `+=`, `-=`, `++` e `--`.

Em C, todos os operadores de atribuição possuem a mesma precedência baixa que em Java (inferior aos operadores aritméticos, relacionais e lógicos), e o lado direito é emitido via `emitirExpressao(no->filhos[1], saida)`, preservando a precedência completa da expressão.

### 3.2 Alvos da Atribuição (L-value) e Casos Especiais

- **Identificador simples (`NO_IDENTIFICADOR`)**: emite diretamente `filhos[0]->valor.valor_str`.
- **Acesso indexado a array (`NO_ACESSO_ARRAY`)**: delega a emissão do lado esquerdo para `emitirAcessoArray(filhos[0], saida)`:
  ```java
  vetor[0] = 42;
  vetor[i + 1] += 10;
  ```
  ```c
  vetor[0] = 42;
  vetor[(i + 1)] += 10;
  ```
- **Leitura de `Scanner` em atribuição (`x = sc.nextInt();`)**: quando `filhos[1]` é um nó `NO_LEITURA_SCANNER` (direto ou encapsulado em `NO_EXPRESSAO`), delega diretamente para `emitirScannerLeitura(leitura, saida, nivel)`.
- **Incremento e decremento como instrução (`NO_INCREMENTO`, `NO_DECREMENTO`, `NO_INCREMENTO_ANTES`, `NO_DECREMENTO_ANTES`)**: gera `i++;`, `i--;`, `++i;` e `--i;` devidamente indentados.

---

## 4. Tradução de Blocos de Escopo (`emitirBloco` — Issue 10.10)

A função `void emitirBloco(NoAST *no, FILE *saida, int nivel)` traduz nós `NO_BLOCO` para blocos delimitados por `{` e `}` em C, preservando o isolamento de escopo (incluindo *shadowing* de variáveis locais) e o recuo progressivo de indentação:

1. Emite `{` no recuo `nivel` (`emitirLinha(saida, nivel, "{")`).
2. Percorre todos os comandos filhos (`no->filhos[i]`) chamando o orquestrador geral `emitirInstrucao(no->filhos[i], saida, nivel + 1)`.
3. Emite `}` no recuo `nivel` (`emitirLinha(saida, nivel, "}")`), garantindo chaves sempre balanceadas, inclusive para blocos vazios (`{\n}\n`).

### Exemplo Prático com Blocos Aninhados e Shadowing

Código Java:

```java
{
    int x = 10;
    {
        int x = 20;
        {
            int x = 30;
            x *= 2;
        }
        x -= 5;
    }
    x += 1;
}
```

Código C gerado:

```c
{
	int x = 10;
	{
		int x = 20;
		{
			int x = 30;
			x *= 2;
		}
		x -= 5;
	}
	x += 1;
}
```

---

## 5. Integração e Prevenção de Conflitos

- **Escopo isolado**: somente `src/codegen/codegen_decl.c` foi modificado na árvore de código-fonte (compartilhado apenas entre as Issues 10.9 e 10.10).
- **Dependências externas via `codegen.h`**:
  - `emitirIndentacao`, `emitirLinha` e `emitirInstrucao` (`src/codegen/codegen_main.c` — Kelyton de Lucas)
  - `emitirExpressao` (`src/codegen/codegen_expr.c` — Pedro Augusto)
  - `emitirScannerLeitura` e `emitirAcessoArray` (`src/codegen/codegen_io.c` — Natan França)
- **Nota de integração com `codegen.h`**: para evitar conflitos durante o desenvolvimento paralelo, `src/codegen/codegen.h` não foi alterado; na etapa de integração final, basta adicionar o protótipo `void emitirBloco(NoAST *no, FILE *saida, int nivel);` em `codegen.h`.
- **Nota de integração com `codegen_io.c`**: esta implementação segue exatamente o contrato declarado em `src/codegen/codegen.h` (`emitirScannerLeitura(NoAST *no, FILE *saida, int nivel)`). Caso a integração final dos módulos de I/O adote assinatura com passagem explícita do identificador de destino (ex.: `emitirScannerLeituraEm(leitura, nome, saida, nivel)`), o ajuste fica restrito às chamadas de delegação em `src/codegen/codegen_decl.c`.

---

## 6. Testes

### Casos de Teste Java (`tests/codegen/`)
- **Issue 10.9 (Tipos e Declarações):**
  - `tests/codegen/valido_decl_primitivos_todos.java`: declarações de `byte`, `short`, `int`, `long`, `float`, `double`, `char`, `boolean` e `String`.
  - `tests/codegen/valido_decl_constante_final.java`: constantes `final` globais e locais com diferentes tipos.
  - `tests/codegen/valido_decl_multiplas_inicializadas.java`: declarações múltiplas com variáveis inicializadas, não inicializadas e constantes `final`.
  - `tests/codegen/valido_decl_scanner_leitura.java`: declaração de `Scanner` e inicialização de variáveis com `nextInt()`, `nextDouble()` e `nextLine()`.
- **Issue 10.10 (Atribuições e Blocos de Escopo):**
  - `tests/codegen/valido_atribuicao_simples.java`: atribuições simples com precedência de expressões e múltiplos tipos.
  - `tests/codegen/valido_atribuicoes_compostas_todas.java`: atribuições compostas (`+=`, `-=`), incrementos/decrementos e operações aritméticas.
  - `tests/codegen/valido_blocos_aninhados_shadowing.java`: blocos `{ ... }` aninhados em múltiplos níveis com *shadowing* de variáveis locais.

### Testes Unitários em C
- `tests/codegen/teste_decl_unitario.c`: valida `traduzirTipoC` e `emitirDeclaracaoVariavel` (36 verificações).
- `tests/codegen/teste_atrib_bloco_unitario.c`: valida `emitirAtribuicao` (todos os 6 operadores `=`, `+=`, `-=`, `*=`, `/=`, `%=`, acesso a array, incremento/decremento e leitura de Scanner) e `emitirBloco` (blocos vazios e aninhamento em 3 níveis com *shadowing* — 22 verificações).

### Como Compilar e Executar

```bash
# Validar todos os casos Java no parser e analisador semantico
make parser
for f in tests/codegen/valido_decl_*.java tests/codegen/valido_atribuic*.java tests/codegen/valido_blocos_*.java; do
    ./build/parser --semantico < "$f" > /dev/null
done

# Compilar e executar as suites unitarias com warnings estritos
gcc -Wall -Wextra -Werror -Isrc -Isrc/ast -Isrc/semantic -Isrc/codegen \
    src/ast/ast.c src/codegen/codegen_main.c src/codegen/codegen_decl.c \
    tests/codegen/teste_decl_unitario.c -o build/teste_decl_unitario
./build/teste_decl_unitario

gcc -Wall -Wextra -Werror -Isrc -Isrc/ast -Isrc/semantic -Isrc/codegen \
    src/ast/ast.c src/codegen/codegen_main.c src/codegen/codegen_decl.c \
    tests/codegen/teste_atrib_bloco_unitario.c -o build/teste_atrib_bloco_unitario
./build/teste_atrib_bloco_unitario
```
