# Compiladores

Este projeto implementa um conversor de um subconjunto da linguagem Java para
C procedural, sem o modelo de orientacao a objetos. A proposta e receber um
programa escrito na sintaxe definida para o subconjunto de Java e, nas etapas
seguintes, gerar um programa equivalente em C.

O projeto ainda esta em desenvolvimento. Nesta etapa foram implementadas
somente as fases lexica e sintatica: o Flex identifica tokens e o Bison
verifica se a entrada segue a gramatica do projeto. A traducao efetiva para C,
assim como analise semantica, verificacao de tipos e geracao de codigo, ainda
serao implementadas.

Atualmente, a gramatica reconhece a estrutura de um programa com uma classe
de nivel superior, atributos, metodos, parametros, blocos e comandos de
controle de fluxo. Ela inclui construcoes como `if`, `else`, `for`, `while`,
`do while`, `switch`, `break` e `return`, mas isso representa apenas o suporte
sintatico necessario para as proximas fases do conversor.

## Organizacao do projeto

- `lexer/`: regras do analisador lexico escrito em Flex.
- `parser/`: gramatica sintatica escrita em Bison.
- `enum/`: definicoes auxiliares de tokens.
- `src/`: codigo C de apoio ao compilador.
- `tests/`: casos positivos e negativos organizados por issue.
- `docs/grammar/`: documentacao das regras implementadas.
- `build/`: arquivos gerados e o executavel do parser.

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

### Compilar o parser

Gera e compila os arquivos do Flex e Bison no binário `build/parser`:

```bash
make
# ou: make parser
```

Para remover todos os arquivos gerados:

```bash
make clean
```

### Testar manualmente pelo terminal

Para testar o parser, passe uma entrada com uma classe:

```bash
printf 'class Exemplo { void executar() {} }\n' | ./build/parser
```

Ou execute interativamente (finalize com `Ctrl+D`):

```bash
./build/parser
```

## Estrutura do programa e integracao das issues

A Issue 4 adicionou a estrutura geral do parser: o programa inicia em uma
classe de nivel superior, e essa classe pode conter atributos e metodos. Os
metodos aceitam retorno tipado ou `void`, modificadores, parametros simples e
parametros de array, incluindo a assinatura `public static void main(String[] args)`.

A Issue 3 de controle de fluxo foi integrada posteriormente. Como ela foi
desenvolvida primeiro com uma gramatica cujo ponto de entrada aceitava
comandos diretamente, sua regra `programa` era incompatível com a estrutura
mais geral de classes da Issue 4. A integracao exigiu manter a classe como
raiz do programa e mover os comandos de controle de fluxo para dentro de
`bloco`, que e usado no corpo dos metodos.

Tambem foi necessario separar `declaracao_variavel`, usada para campos da
classe, de `declaracao_local`, usada para declaracoes dentro de blocos e
comandos `for`. Sem essa separacao, o Bison encontrava conflitos entre campos
e variaveis locais.

Os testes originais de controle de fluxo eram snippets de comandos soltos.
Depois da integracao, o formato esperado para novos testes e uma entrada Java
com classe e metodo envolvendo os comandos testados.

### Fora do escopo atual

Esta versao ainda nao implementa:

- classes `abstract`;
- multiplas classes de nivel superior no mesmo arquivo;
- classes aninhadas;
- `private`, `protected` ou `static` em classes de nivel superior;
- validacao semantica de tipos, visibilidade e regras completas da linguagem Java.

Para executar os testes de cada issue:

```bash
make parser
make test ISSUE=3
make test ISSUE=4
```

Os testes da Issue 4 estao alinhados com a estrutura de classe. Os testes
positivos antigos da Issue 3 que ainda forem comandos soltos precisam ser
envolvidos por uma classe e um metodo para usar a gramatica integrada.
