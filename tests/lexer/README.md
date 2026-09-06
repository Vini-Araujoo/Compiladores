# Testes do lexer

Estes testes verificam se o lexer reconhece as palavras reservadas, os operadores, os arrays, a entrada e a saida suportadas pelo projeto.

## Pré-requisito

Execute os comandos a partir da raiz do projeto:

```bash
cd /home/lucas/Desktop/HailMary/Compiladores
```

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

Esse comando apenas executa o lexer. Ele não compara a saída com os arquivos
em `esperado/`.

Por exemplo, se o arquivo de entrada contiver:

```java
int valore = 10;
```

o lexer imprimirá:

```text
lexema: valore | token: 3
```

Isso acontece porque `valore` é um identificador (`ID`). O nome do
identificador aparece na saída, embora o token seja o mesmo para qualquer
variável.

## Executar todos os testes

```bash
./tests/lexer/run-tests.sh
```

O script recompila o lexer, executa cada entrada e compara o resultado com os arquivos em `tests/lexer/esperado/`. Se tudo estiver correto, ele termina com sucesso. Caso contrário, o `diff` mostra a diferença encontrada.

Por exemplo, se `entrada.java` tiver `valore`, mas o arquivo
`esperado/entrada.txt` tiver `valres`, o teste falhará com uma diferença
parecida com esta:

```diff
-lexema: valres | token: 3
+lexema: valore | token: 3
```

Nesse caso, o lexer está funcionando, mas o resultado esperado está diferente
do resultado real. Para o teste passar, o arquivo em `esperado/` precisa
registrar exatamente a saída produzida pelo lexer.

Para fazer uma comparação manual, use:

```bash
./build/lexico < tests/lexer/entrada.java > /tmp/saida.txt
diff -u tests/lexer/esperado/entrada.txt /tmp/saida.txt
```

## Organização

- `palavras-reservadas.java`: tipos, controle de fluxo, modificadores e `new`.
- `operadores.java`: operadores simples, compostos e relacionais.
- `entrada.java`: arrays, `System.out`, `Scanner` e métodos `next...`.
- `esperado/`: saídas esperadas geradas pelo lexer.
- `run-tests.sh`: comando automatizado dos testes.
