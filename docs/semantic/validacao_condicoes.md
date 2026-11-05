# Validação de condições de controle

As condições de `if`, `while`, `do-while` e `for` devem resultar no tipo `boolean`. Uma expressão inteira, real ou textual não é aceita como condição.

## Posições na AST

As expressões de condição ficam diretamente nos nós de controle, sem um nó intermediário `NO_CONDICAO`:

| Nó | Filho que contém a condição |
| --- | --- |
| `NO_IF` | `filhos[0]` |
| `NO_WHILE` | `filhos[0]` |
| `NO_DO_WHILE` | `filhos[1]` |
| `NO_FOR` | `filhos[1]` |

O cabeçalho de `for` pode omitir a condição (`for (;;)`). Nesse caso, `filhos[1]` é `NULL` e não há expressão para validar.

## Regra semântica

A análise percorre a expressão para resolver seus identificadores e consulta `inferirTipoExpressao`. Se o resultado não for `TIPO_BOOLEAN`, registra um erro semântico na linha da expressão com a mensagem:

```text
Erro semantico na linha N: condicao deve ser booleana.
```

São aceitos literais booleanos, variáveis booleanas e expressões relacionais ou lógicas que resultem em booleano. Condições de tipo `int`, `float`, `double` ou `String` são rejeitadas.

## Testes

Os testes ficam em `tests/semantic/` e são executados por `make test-semantic`:

- `valido_condicao_relacional_boolean.java`: cobre variável booleana, operador lógico e condições relacionais em `if`, `while`, `do-while` e `for`.
- `valido_condicao_literal_boolean.java`: cobre literais booleanos nos quatro tipos de controle.
- `invalido_condicao_if_inteiro.java`: rejeita inteiro em `if`.
- `invalido_condicao_while_string.java`: rejeita `String` em `while`.
- `invalido_condicao_do_while_inteiro.java` e `invalido_condicao_for_inteiro.java`: verificam a rejeição nos outros dois nós de repetição.
- `invalido_condicao_float.java` e `invalido_condicao_double.java`: rejeitam condições numéricas reais.

O teste do literal booleano também inclui `for (;;)`, que não possui expressão condicional e deve ser aceito.
