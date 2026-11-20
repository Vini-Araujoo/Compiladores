# Tradução de I/O para C

Este documento descreve como o gerador de código traduz os comandos de saída
(`System.out.println` e `System.out.print`) e os métodos de leitura do `Scanner`
para C. O código está em `src/codegen/codegen_io.c`.

## Saída: `println` e `print`

A máscara do `printf` depende do tipo da expressão. O `println` acrescenta `\n`
ao final da string de formato; o `print` não acrescenta.

| Tipo Java | Máscara | Exemplo gerado (`println`) |
| --- | --- | --- |
| `int`, `short`, `byte` | `%d` | `printf("%d\n", x);` |
| `long` | `%lld` | `printf("%lld\n", x);` |
| `float`, `double` | `%lf` | `printf("%lf\n", x);` |
| `String` | `%s` | `printf("%s\n", s);` |
| `char` | `%c` | `printf("%c\n", c);` |
| `boolean` | `%s` | `printf("%s\n", (b) ? "true" : "false");` |

`System.out.println();` sem argumento gera `printf("\n");`.

## Entrada: métodos do `Scanner`

| Método Java | Máscara | Exemplo gerado | Usa `&` |
| --- | --- | --- | --- |
| `nextInt()` | `%d` | `scanf("%d", &x);` | sim |
| `nextDouble()` | `%lf` | `scanf("%lf", &x);` | sim |
| `nextFloat()` | `%f` | `scanf("%f", &x);` | sim |
| `nextLong()` | `%lld` | `scanf("%lld", &x);` | sim |
| `nextShort()` | `%hd` | `scanf("%hd", &x);` | sim |
| `nextByte()` | `%hhd` | `scanf("%hhd", &x);` | sim |
| `nextBoolean()` | `%d` | `scanf("%d", &x);` | sim |
| `next()` | `%s` | `scanf("%s", s);` | não |
| `nextLine()` | ` %[^\n]` | `scanf(" %[^\n]", s);` | não |

`next` e `nextLine` leem para um vetor de `char`, que já decai para ponteiro,
por isso não levam `&`.

## Instanciação do `Scanner`

`Scanner sc = new Scanner(System.in);` não gera declaração de variável, porque a
entrada padrão (`stdin`) já está aberta em C. O gerador emite apenas um
comentário:

```c
/* Scanner sc = new Scanner(System.in); */
```

## Contextos de emissão

| Java | C gerado |
| --- | --- |
| `int x = sc.nextInt();` | `int x;` seguido de `scanf("%d", &x);` |
| `String s = sc.nextLine();` | `char s[256];` seguido de `scanf(" %[^\n]", s);` |
| `x = sc.nextInt();` | `scanf("%d", &x);` |

A declaração da variável é responsabilidade do emissor de declarações. O módulo
de I/O expõe `emitirScannerLeituraEm(no, destino, saida, nivel)`, que recebe o
nome da variável de destino e emite o `scanf`.

## Limitações e diferenças de comportamento

- **Leitura sem atribuição:** `sc.nextInt();` como comando isolado não é
  suportado e gera um erro de geração, pois não há variável de destino.
- **`nextLine` após leitura numérica:** em Java, `nextLine()` depois de
  `nextInt()` devolve o resto da linha (normalmente vazio). Em C, a máscara
  ` %[^\n]` descarta os espaços em branco iniciais e lê a próxima linha de
  texto.
- **`double` e `float`:** o `printf` com `%lf` imprime seis casas decimais
  (`3.000000`), enquanto o Java imprime `3.0`.
- **`boolean`:** o `scanf` lê um inteiro (`%d`). Em C, `bool` é numérico, e
  qualquer valor diferente de zero é verdadeiro.
- **Buffers de `String`:** a leitura de `next` e `nextLine` assume um vetor de
  256 posições, sem verificação de estouro.
- **Aviso de `scanf`:** com `-O2`, o GCC pode emitir `warn_unused_result` por
  não tratar o retorno de `scanf`. Com `-Wall -Wextra` sem otimização, o código
  gerado compila sem warnings.