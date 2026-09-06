## Analisador lexico

## Regras atuais

O lexer reconhece palavras-chave (`if`, `else`, `while`, `for`, `return`,
`byte`, `short`, `int`, `long`, `boolean`, `char`, `float` e `double`),
identificadores, numeros inteiros, operadores (`=`, `==`, `!=`, `<=`, `>=`,
`+`, `-`, `*` e `/`) e delimitadores (`;`, `,`, `(`, `)`, `{` e `}`).

Comentarios `// ...` e `/* ... */`, espacos, tabs e quebras de linha sao
ignorados. As regras dos operadores com dois caracteres aparecem antes das
regras menores para que sejam reconhecidos inteiros.

### Exemplo 1: declaracao simples

Entrada:

```text
int contador = 42;
```

Saida atual:

```text
lexema: int
lexema: contador
lexema: =
lexema: 42
lexema: ;
```

### Exemplo 2: palavras-chave e comentario

Entrada:

```text
// verifica o limite
if (contador >= 10) {
  return contador + 1;
}
```

Saida atual:

```text
lexema: if
lexema: (
lexema: contador
lexema: >=
lexema: 10
lexema: )
lexema: {
lexema: return
lexema: contador
lexema: +
lexema: 1
lexema: ;
lexema: }
```

### Exemplo 3: caractere desconhecido

Entrada:

```text
abc ! 42
```

Saida atual:

```text
lexema: abc
Erro: caractere desconhecidolexema: 42
```

O caractere `!` nao e um operador isolado nas regras atuais. Ele gera uma
mensagem de erro, mas o lexer continua processando a entrada.

## Como rodar

Os comandos devem ser executados na raiz do projeto:

### Compilar o lexer

Gera o codigo do Flex e cria o executavel `build/lexico`:

```bash
make lexer
```

### Compilar o parser

Gera e compila os arquivos do Bison:

```bash
make parser
```

### Compilar o projeto inteiro

Executa os alvos do lexer e do parser:

```bash
make
```

Para remover todos os arquivos gerados:

```bash
make clean
```

### Testar pelo terminal

Passe a entrada pela entrada padrao:

```bash
echo "int contador = 42;" | ./build/lexico
```

Ou digite varias linhas manualmente:

```bash
./build/lexico
```

Finalize a entrada com `Ctrl+D` no Linux.
