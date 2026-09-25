# Documentação Sintática: Controle de Fluxo, Desvios e Blocos

- **Issue:** Issue 3
- **Responsável:** morettipdr
- **Branch:** `feat/issue-3-controle-fluxo`

---

## 1. Descrição Geral

Esta issue implementa as estruturas de controle de fluxo da linguagem no
parser Bison: blocos, condicionais, seleção por casos, laços, `break` e
`return`.

As regras de expressão, declaração e atribuição atuam como stubs mínimos para
permitir testes isolados da Issue 3. Elas serão integradas às regras completas
das demais issues posteriormente.

## 2. Regras Sintáticas

### Estrutura de programa, comandos e blocos

```bnf
programa        ::= { comando }
lista_comandos  ::= { comando }
bloco           ::= "{" lista_comandos "}"

comando ::= declaracao_variavel
           | atribuicao
           | condicional
           | repeticao
           | comando_salto
           | bloco
```

Um bloco aceita zero ou mais comandos. Não há comando vazio isolado (`;`) na
gramática atual.

### Stubs de integração

```bnf
expressao             ::= ID | NUMERO
declaracao_variavel   ::= "int" ID ";"
                       | "int" ID "=" expressao ";"
atribuicao            ::= ID "=" expressao ";"
                       | "++" ID ";" | "--" ID ";"
                       | ID "++" ";" | ID "--" ";"
```

### Condições e prioridade lógica

```bnf
condicao        ::= condicao_ou
condicao_ou     ::= condicao_ou "||" condicao_e | condicao_e
condicao_e      ::= condicao_e "&&" condicao_base | condicao_base
condicao_base   ::= "(" condicao ")"
                  | "true" | "false"
                  | expressao comparador expressao
                  | expressao
comparador      ::= "==" | "!=" | "<" | ">" | "<=" | ">="
```

A estrutura das regras define a prioridade: parênteses e comparações possuem
maior prioridade, `&&` vem em seguida e `||` possui a menor prioridade.

### Condicionais e dangling else

```bnf
comando_if ::= "if" "(" condicao ")" comando
             | "if" "(" condicao ")" comando "else" comando
```

O parser usa as diretivas abaixo:

```yacc
%nonassoc LOWER_THAN_ELSE
%nonassoc TOKEN_ELSE
```

A alternativa de `if` sem `else` recebe `%prec LOWER_THAN_ELSE`. Como
`TOKEN_ELSE` possui prioridade maior, um `else` é associado ao `if` mais
próximo, eliminando a ambiguidade do *dangling else*.

### Switch

```bnf
comando_switch  ::= "switch" "(" expressao ")" "{"
                    casos_switch default_switch "}"
casos_switch    ::= { case_switch }
case_switch     ::= "case" expressao ":" lista_comandos
default_switch  ::= [ "default" ":" lista_comandos ]
```

O `default` é opcional e, quando presente, aparece após os `case`. Cada caso
pode conter vários comandos, inclusive `break;`.

### Laços de repetição

```bnf
repeticao        ::= comando_while | comando_do_while | comando_for
comando_while    ::= "while" "(" condicao ")" bloco
comando_do_while ::= "do" bloco "while" "(" condicao ")" ";"
comando_for      ::= "for" "(" atribuicao_for condicao_for incremento_for ")" bloco

atribuicao_for ::= ";"
                  | ID "=" expressao ";"
                  | declaracao_variavel
condicao_for   ::= ";" | condicao ";"
incremento_for ::= ε
                  | ID "=" expressao
                  | ID "++" | ID "--"
                  | "++" ID | "--" ID
```

Os três segmentos do cabeçalho do `for` podem estar ausentes, portanto
`for (;;) { ... }` é válido. Por decisão do escopo atual, `while`, `do-while`
e `for` exigem um `bloco` entre chaves como corpo; comandos sem chaves não são
aceitos nesses laços.

### Saltos e retorno

```bnf
comando_salto  ::= "break" ";" | comando_return
comando_return ::= "return" ";"
                  | "return" expressao ";"
```

`break;` é reconhecido como comando. A validação semântica de contexto — por
exemplo, impedir `break` fora de laços ou `switch`, e `return` fora de métodos
— permanece fora do escopo desta issue.

## 3. Integração

O não-terminal `bloco` está disponível para consumo pela Issue 4. As regras
de controle são centralizadas em `condicional` (`comando_if` e
`comando_switch`), `repeticao` (`comando_while`, `comando_do_while` e
`comando_for`) e `comando_salto`.

## 4. Testes e Validação

Execute toda a suíte da Issue 3 com:

```bash
make test ISSUE=3
```

O script compila o parser, exige código `0` para arquivos `valido_*.java` e
erro sintático/código diferente de `0` para `invalido_*.java`.

Resultado validado:

```text
Resumo: 26 passou, 0 falhou.
```

## Casos de Teste Validados

- [x] `tests/parser/issue_3/valido_if_else.java`
- [x] `tests/parser/issue_3/valido_while_e_dowhile.java`
- [x] `tests/parser/issue_3/valido_for.java`
- [x] `tests/parser/issue_3/valido_switch.java`
- [x] `tests/parser/issue_3/valido_aninhamento_e_retorno.java`
- [x] `tests/parser/issue_3/invalido_if_sem_parenteses.java`
- [x] `tests/parser/issue_3/invalido_while_sem_condicao.java`
- [x] `tests/parser/issue_3/invalido_do_while_sem_ponto_virgula.java`
- [x] `tests/parser/issue_3/invalido_for_sem_delimitadores.java`
