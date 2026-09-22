# Documentacao Sintatica: Estrutura do Programa, Classes e Metodos

- **Issue:** Issue 4
- **Responsável:** Kelyton de Lucas Moraes Santos
- **Branch:** `feat/issue-4-estrutura-programa`

---

## 1. Descricao Geral

A Issue 4 implementa a estrutura externa do programa Java. O ponto de entrada
`programa` representa uma unica classe de nivel superior, que pode conter
atributos e metodos.

A classe e reconhecida com os modificadores de nivel superior previstos nesta
etapa:

```text
class ID { corpo_classe }
final class ID { corpo_classe }
public class ID { corpo_classe }
public final class ID { corpo_classe }
```

Modificadores como `private`, `protected` e `static` nao sao aceitos antes de
uma classe de nivel superior.

## 2. Regras Implementadas

As principais regras da estrutura sao:

```bison
programa:
		declaracao_classe
;

corpo_classe:
			%empty
		| corpo_classe membro_classe
;

membro_classe:
			declaracao_variavel
		| declaracao_metodo
;

declaracao_metodo:
			modificadores tipo ID '(' lista_parametros ')' bloco
		| modificadores TOKEN_VOID ID '(' lista_parametros ')' bloco
;
```

Os modificadores de atributos e metodos sao opcionais e podem ser combinados:

```text
public
private
protected
static
final
```

Os parametros podem ser simples ou arrays:

```bison
parametro:
			tipo ID
		| tipo '[' ']' ID
;
```

Por isso, a assinatura abaixo e reconhecida:

```java
public static void main(String[] args) {
}
```

## 3. Integracao com Controle de Fluxo

Inicialmente, o corpo de metodo era um stub que aceitava apenas `{}`. Depois
da integracao com a Issue 3, `bloco` passou a conter uma lista de comandos:

```bison
bloco:
		TOKEN_ABRE_CHAVE lista_comandos TOKEN_FECHA_CHAVE
;
```

Os comandos de controle de fluxo ficam dentro dos metodos, incluindo
atribuicoes, condicionais, lacos, `break`, `return` e blocos aninhados.

As declaracoes de campos da classe e as declaracoes locais de comandos foram
separadas em `declaracao_variavel` e `declaracao_local`. Essa separacao evita
que o Bison confunda um atributo da classe com uma variavel usada dentro de um
`for` ou de outro bloco.

## 4. Stubs e Dependencias

Durante o desenvolvimento isolado, `tipo`, `declaracao_variavel` e `bloco`
foram usados como pontos de integracao entre as issues. Com a uniao das
branches, o stub de `bloco` foi substituido pela regra de comandos da Issue 3.

O parser continua usando as regras de tipos e declaracoes necessarias para os
testes desta etapa. A evolucao dessas regras pode ser feita pelas issues
responsaveis por tipos, expressoes e declaracoes.

## 5. Escopo e Limitacoes

Ficaram fora desta etapa:

- classes `abstract`;
- multiplas classes de nivel superior no mesmo programa;
- classes aninhadas;
- modificadores `private`, `protected` e `static` em classes de nivel superior;
- semantica de Java, como verificacao de tipos, visibilidade duplicada ou
  regras de inicializacao;
- corpos completos de metodos alem dos comandos cobertos pela Issue 3.

O parser faz analise sintatica. A validacao semantica sera tratada em etapas
posteriores.

## 6. Testes

Os testes da Issue 4 ficam em `tests/parser/issue_4/` e podem ser executados
com:

```bash
make parser
make test ISSUE=4
```

Ou diretamente:

```bash
bash tests/parser/run-tests.sh 4
```

Na validacao atual, os 12 testes da Issue 4 passam.
