# Testes do lexer

Estes testes verificam se o lexer reconhece as palavras reservadas, os operadores, os arrays, a entrada e a saida suportadas pelo projeto.

## Pré-requisito

Execute os comandos a partir da raiz do projeto

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
lexema: valore | token: 3
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

- `run-tests.sh`: comando automatizado dos testes.

# Testes antigos do lexer

Esta pasta contém entradas e saídas de uma versão anterior do projeto, quando
o lexer era executado diretamente pelo binário `build/lexico` e imprimia cada
lexema e seu token.

## Estado atual

O ponto de entrada atual é o parser:

```bash
make
./build/parser < arquivo.java
```

O lexer agora é chamado internamente pelo parser e não imprime mais tokens.
Por isso, os arquivos `esperado/*.txt` e o script `run-tests.sh` desta pasta
não são compatíveis com a estrutura atual. O comando abaixo falha porque o
alvo `lexer` e o executável `build/lexico` não existem mais:

```bash
./tests/lexer/run-tests.sh
```

Além disso, as entradas `.java` desta pasta contêm vários comandos e são
maiores do que a gramática atual do parser, que neste momento aceita apenas
uma declaração no formato:

```text
int identificador = numero;
```

Exemplo funcional:

```bash
printf 'int x = 5;\n' | ./build/parser
```

## Próximos testes

Quando a gramática do parser estiver implementada, estas entradas podem ser
convertidas em testes sintáticos. Se a intenção for continuar testando apenas
o lexer, será necessário criar um pequeno programa de teste que inclua
`build/parser.tab.h`, chame `yylex()` e imprima os tokens para comparação com
os arquivos em `esperado/`.
