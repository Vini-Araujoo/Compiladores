# Tradução de arrays para C

Este documento descreve como o gerador de código traduz arrays unidimensionais
de Java para C e qual é o modelo de memória adotado. O código está em
`src/codegen/codegen_io.c`.

## Nós da AST envolvidos

| Nó | Filho 0 | Filho 1 | `tipo_dado` |
| --- | --- | --- | --- |
| `NO_INSTANCIACAO_ARRAY` | expressão do tamanho | (não possui) | tipo base: `TIPO_INT`, `TIPO_DOUBLE` ou `TIPO_STRING` |
| `NO_ACESSO_ARRAY` | `NO_IDENTIFICADOR` com o nome do array | expressão do índice | (não utilizado) |

O mesmo `NO_ACESSO_ARRAY` serve como L-value (`vetor[i] = x;`) e como R-value
(`x = vetor[i];`). A diferença está só na posição do nó na árvore.

## Instanciação: `emitirInstanciacaoArray`

Gera uma **expressão** (sem indentação, sem `;` e sem quebra de linha), que
quem chama encaixa no lugar certo, normalmente do lado direito de uma atribuição.

| Java | C gerado |
| --- | --- |
| `new int[10]` | `(int*)malloc((10) * sizeof(int))` |
| `new double[n + 1]` | `(double*)malloc((n + 1) * sizeof(double))` |
| `new String[5]` | `(char**)malloc((5) * sizeof(char*))` |

O tamanho vem entre parênteses para preservar a precedência quando é uma
expressão como `n + 1`. A forma estática (`int v[10]`) não é usada, porque o nó
de instanciação não guarda o nome da variável e um array estático precisa dele.
`stdlib.h` já é incluído pelo cabeçalho do código gerado.

## Acesso indexado: `emitirAcessoArray`

Também gera uma **expressão**: o nome do array, `[`, o índice emitido por
`emitirExpressao` e `]`.

| Java | C gerado |
| --- | --- |
| `vetor[0]` | `vetor[0]` |
| `vetor[i + 1]` | `vetor[i + 1]` |
| `vetor[outro[0]]` | `vetor[outro[0]]` |

## Contextos de emissão

| Java | C gerado |
| --- | --- |
| `vetor = new int[10];` | `vetor = (int*)malloc((10) * sizeof(int));` |
| `vetor[i] = i * 2;` | `vetor[i] = i * 2;` |
| `x = vetor[i];` | `x = vetor[i];` |

A atribuição, a indentação, o `;` e a quebra de linha são responsabilidade de
`emitirAtribuicao` (`codegen_decl.c`). Ela deve chamar `emitirAcessoArray`
quando o filho 0 da atribuição for um `NO_ACESSO_ARRAY`, e
`emitirInstanciacaoArray` (diretamente ou via `emitirExpressao`) quando o lado
direito for um `NO_INSTANCIACAO_ARRAY`.

## Modelo de memória

- O array é um **ponteiro** para um bloco alocado no heap com `malloc`. O tipo C
  da variável é `int*`, `double*` ou `char**`.
- O tamanho **não é guardado**: o C gerado não tem equivalente de `vetor.length`.
- O bloco **não é liberado**: o gerador não emite `free`, então a memória só é
  devolvida quando o programa termina.
- Os elementos **não são inicializados**. Em Java, um `new int[10]` vem zerado;
  em C, o conteúdo é indeterminado até a primeira escrita.
- Em `new String[n]`, apenas o vetor de ponteiros é alocado. Cada elemento é um
  `char*` que ainda precisa apontar para uma string.
- O índice **não é verificado**: acessar fora dos limites tem comportamento
  indefinido em C, em vez de lançar exceção como em Java.

## Limitações conhecidas

- **Sem declaração de variável de array.** A gramática atual não aceita
  `int[] v = new int[10];` nem `int v[10];`: o tipo `int[]` só existe em
  parâmetros de método. Por isso nenhum módulo emite `int *vetor;`, e o programa
  C completo gerado a partir de um array local não compila até a gramática
  aceitar essa declaração. Os testes usam `vetor = new int[10];`, como os testes
  da AST.
- **Tipo do elemento desconhecido.** O símbolo guarda só `TIPO_ARRAY`, e o nó
  `NO_ACESSO_ARRAY` não tem `tipo_dado`. Por isso `System.out.println(v[i])`
  não consegue escolher a máscara do `printf` e gera um erro de geração.
- **Tipos base:** apenas `int`, `double` e `String`, os mesmos que o parser
  aceita em `new tipo[...]`.
- **Somente uma dimensão.** Não há suporte a arrays multidimensionais.
- **Tamanho negativo ou zero** e **falha do `malloc`** não são tratados.
