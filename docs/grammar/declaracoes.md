# Tipos, declarações e atribuições

Esta documentação descreve as regras implementadas no módulo sintático do
parser (`parser/parser.y`) para a Issue 1. O parser verifica a forma da
entrada; ele ainda não faz análise semântica, inferência de tipos ou validação
de escopo.

## Tipos reconhecidos

A não-terminais `tipo` representa os tipos primitivos e `String` aceitos nas
declarações e nos parâmetros de métodos:

```bnf
<tipo> ::= "byte"
		  | "short"
		  | "int"
		  | "long"
		  | "boolean"
		  | "char"
		  | "float"
		  | "double"
		  | "String"
```

Os nomes acima são tokens reconhecidos pelo lexer. Um identificador usado como
tipo, um tipo inexistente ou uma combinação de tokens que não aparece nessa
produção causa erro sintático.

## Modificadores

Campos da classe podem receber os modificadores gerais abaixo. Para variáveis
locais, a gramática permite apenas `final` como modificador específico:

```bnf
<modificadores> ::= vazio
				  | <modificadores> <modificador>

<modificador> ::= "public"
				| "private"
				| "protected"
				| "static"
				| "final"

<modificador_variavel> ::= vazio
						  | "final"
```

Na prática, a produção de campo usa `<modificadores>`, enquanto a produção de
declaração local usa `<modificador_variavel>`.

## Declarações de variáveis

Uma declaração termina obrigatoriamente com ponto e vírgula. O mesmo tipo pode
introduzir um ou mais declaradores separados por vírgula, e cada declarador
pode ter ou não uma inicialização:

```bnf
<declaracao_variavel> ::= <modificadores> <tipo>
						  <lista_declaracoes> ";"

<declaracao_local> ::= <modificador_variavel> <tipo>
					   <lista_declaracoes> ";"

<lista_declaracoes> ::= <item_declaracao>
					  | <lista_declaracoes> "," <item_declaracao>

<item_declaracao> ::= <identificador>
					| <identificador> "=" <expressao>
```

`<declaracao_variavel>` é usada como membro da classe, portanto representa
atributos. `<declaracao_local>` é usada dentro de blocos de métodos e também é
uma das formas de comando aceitas pelo parser.

Exemplos aceitos:

```java
int contador;
double preco = 10;
final int limite = 100;
int x, y, z;
int a = 1, b, c = 3;
```

O não-terminal de expressões usado nesta etapa é deliberadamente pequeno:

```bnf
<expressao> ::= <identificador>
			  | <numero>
```

Assim, inicializações com identificadores ou números inteiros são reconhecidas.
Expressões aritméticas, literais booleanos, caracteres, strings e chamadas de
método ainda não fazem parte dessa produção.

## Atribuições

As atribuições são comandos e, por isso, também exigem ponto e vírgula:

```bnf
<atribuicao> ::= <identificador> "=" <expressao> ";"
			   | "++" <identificador> ";"
			   | "--" <identificador> ";"
			   | <identificador> "++" ";"
			   | <identificador> "--" ";"
			   | <identificador> "+=" <expressao> ";"
			   | <identificador> "-=" <expressao> ";"
```

Portanto, o parser aceita atribuição simples, soma e subtração com atribuição,
além das formas prefixada e pós-fixada de incremento e decremento:

```java
x = 5;
contador += 1;
saldo -= 10;
x++;
y--;
++x;
--y;
```

As regras verificam apenas a estrutura. Por exemplo, elas não confirmam se a
variável foi declarada antes do uso, se o valor é compatível com o tipo ou se
uma variável `final` está sendo alterada. Essas verificações pertencem à etapa
semântica, ainda não implementada.

## Integração com o restante do parser

O ponto de entrada exige uma classe. Campos são reconhecidos em `corpo_classe`
por meio de `membro_classe`, enquanto declarações locais e atribuições são
reconhecidas em `lista_comandos` dentro dos blocos de métodos:

```bnf
<membro_classe> ::= <declaracao_variavel>
				  | <declaracao_metodo>

<comando> ::= <declaracao_local>
			| <atribuicao>
			| <condicional>
			| <repeticao>
			| <comando_salto>
			| <bloco>
```

Essa separação evita misturar a declaração de atributos com a declaração de
variáveis locais e permite que ambas usem a mesma estrutura de lista de
declaradores.

## Casos de teste da Issue 1

Os quatro arquivos atualmente presentes em `tests/parser/issue_1/` cobrem as
formas implementadas:

- [x] `tests/parser/issue_1/valido_primitivos.java`
- [x] `tests/parser/issue_1/valido_inicializacoes.java`
- [x] `tests/parser/issue_1/valido_multiplas_declaracoes.java`
- [x] `tests/parser/issue_1/valido_atribuicoes.java`

Os casos inválidos abaixo verificam rejeições sintáticas relacionadas a
declarações e atribuições:

- [x] `tests/parser/issue_1/invalido_sem_ponto_virgula.java` — declaração sem `;`
- [x] `tests/parser/issue_1/invalido_tipo_inexistente.java` — tipo não reconhecido
- [x] `tests/parser/issue_1/invalido_atribuicao_sem_expressao.java` — atribuição sem expressão
- [x] `tests/parser/issue_1/invalido_virgula_sobrando.java` — vírgula sem declarador seguinte
