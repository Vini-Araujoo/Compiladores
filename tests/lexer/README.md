# Testes do lexer

Estes testes verificam se o lexer reconhece as palavras reservadas, os operadores, os arrays, a entrada e a saida suportadas pelo projeto.

## Pré-requisito

Execute os comandos a partir da raiz do projeto:

É necessário ter `flex`, `gcc` e `make` instalados.

## Compilar o lexer

```bash
make lexer
```

## Executar um teste manualmente

```bash
./build/lexico < tests/lexer/palavras-reservadas.java
./build/lexico < tests/lexer/operadores.java
./build/lexico < tests/lexer/entrada.java
```

A saída mostra cada lexema e o número do token retornado.

## Executar todos os testes

```bash
./tests/lexer/run-tests.sh
```

O script recompila o lexer, executa cada entrada e compara o resultado com os arquivos em `tests/lexer/esperado/`. Se tudo estiver correto, ele termina com sucesso. Caso contrário, o `diff` mostra a diferença encontrada.

## Organização

- `palavras-reservadas.java`: tipos, controle de fluxo, modificadores e `new`.
- `operadores.java`: operadores simples, compostos e relacionais.
- `entrada.java`: arrays, `System.out`, `Scanner` e métodos `next...`.
- `esperado/`: saídas esperadas geradas pelo lexer.
- `run-tests.sh`: comando automatizado dos testes.
