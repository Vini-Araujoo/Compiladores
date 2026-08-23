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

### Gerar e compilar o lexer

Dentro desta pasta, execute:

```bash
flex hello.l
gcc lex.yy.c -o lexico
```

Isso gera `lex.yy.c` e cria o executavel `lexico`.

Esses dois arquivos sao gerados e estao listados no `.gitignore`.

### Testar pelo terminal

Passe a entrada pela entrada padrao:

```bash
echo "int contador = 42;" | ./lexico
```

Ou digite varias linhas manualmente:

```bash
./lexico
```

Finalize a entrada com `Ctrl+D` no Linux.
