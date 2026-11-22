# Tradução de Tipos Java → C e Declaração de Variáveis (Issue 10.9)

Este documento descreve a especificação técnica e a implementação do mapeamento de tipos primitivos, booleanos e strings do Java para a linguagem C, bem como a geração de código C para declarações de variáveis simples, inicializadas, múltiplas, constantes `final` e casos especiais envolvendo `Scanner`.

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

## 3. Integração e Prevenção de Conflitos

- **Escopo isolado**: somente `src/codegen/codegen_decl.c` foi modificado na árvore de código-fonte.
- **Dependências externas via `codegen.h`**:
  - `emitirIndentacao` e `emitirLinha` (`src/codegen/codegen_main.c`)
  - `emitirExpressao` (`src/codegen/codegen_expr.c` — Pedro Augusto)
  - `emitirScannerLeitura` (`src/codegen/codegen_io.c` — Natan França)
- **Nota de integração com `codegen_io.c`**: esta implementação segue exatamente o contrato declarado em `src/codegen/codegen.h` (`emitirScannerLeitura(NoAST *no, FILE *saida, int nivel)`). Caso a integração final dos módulos de I/O adote assinatura com passagem explícita do identificador de destino (ex.: `emitirScannerLeituraEm(leitura, nome, saida, nivel)`), o ajuste fica restrito à única linha de delegação em `emitirDeclaracaoInterna`.

---

## 4. Testes

### Casos de Teste Java (`tests/codegen/`)
- `tests/codegen/valido_decl_primitivos_todos.java`: declarações de `byte`, `short`, `int`, `long`, `float`, `double`, `char`, `boolean` e `String`.
- `tests/codegen/valido_decl_constante_final.java`: constantes `final` globais e locais com diferentes tipos.
- `tests/codegen/valido_decl_multiplas_inicializadas.java`: declarações múltiplas com variáveis inicializadas, não inicializadas e constantes `final`.
- `tests/codegen/valido_decl_scanner_leitura.java`: declaração de `Scanner` e inicialização de variáveis com `nextInt()`, `nextDouble()` e `nextLine()`.

### Teste Unitário em C
- `tests/codegen/teste_decl_unitario.c`: valida todos os mapeamentos de `traduzirTipoC`, todos os tipos em `emitirDeclaracaoVariavel`, propagação de `final`/`const` (via `mods` e `simbolo`), contêineres de declarações múltiplas, formato com filho `NO_IDENTIFICADOR`, casos especiais de `Scanner` e casos defensivos.

### Como Compilar e Executar

```bash
# Validar os casos Java no parser e analisador semantico
make parser
for f in tests/codegen/valido_decl_*.java; do
    ./build/parser --semantico < "$f" > /dev/null
done

# Compilar e executar a suite unitaria com warnings estritos
gcc -Wall -Wextra -Werror -Isrc -Isrc/ast -Isrc/semantic -Isrc/codegen \
    src/ast/ast.c src/codegen/codegen_main.c src/codegen/codegen_decl.c \
    tests/codegen/teste_decl_unitario.c -o build/teste_decl_unitario
./build/teste_decl_unitario
```
