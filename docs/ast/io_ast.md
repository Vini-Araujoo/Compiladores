# AST de Entrada e Saída (I/O)

Este documento descreve a representação na AST dos comandos de saída (`System.out.print` e `System.out.println`) e de entrada via `Scanner` (instanciação e leitura).

## Nós

| Nó | Filho 0 | Campo `valor` | Campo `tipo_dado` |
| --- | --- | --- | --- |
| `NO_PRINT` | Expressão do argumento (`NO_EXPRESSAO`) | (não utilizado) | (não utilizado) |
| `NO_PRINTLN` | Expressão do argumento (`NO_EXPRESSAO`), ausente em `println()` | (não utilizado) | (não utilizado) |
| `NO_INSTANCIACAO_SCANNER` | (não possui) | (não utilizado) | `TIPO_SCANNER` |
| `NO_LEITURA_SCANNER` | Variável do Scanner (`NO_IDENTIFICADOR`) | `valor_str`: nome do método (`nextInt`, `nextLine`...) | (não utilizado) |

Todos os nós registram a linha do código-fonte em `linha`.

### `NO_PRINT` e `NO_PRINTLN`

Produzidos pela regra `comando_print`. `print` exige argumento; `println` aceita argumento vazio, e nesse caso o nó fica sem filhos. A impressão da AST rotula o filho como `argumento`.

### `NO_INSTANCIACAO_SCANNER`

Produzido pela regra `instanciacao` para `new Scanner(System.in)`. É um nó folha: a forma `System.in` é fixa na gramática e não precisa ser guardada.

### `NO_LEITURA_SCANNER`

Produzido pela regra `leitura_scanner` para `variavel.metodo()`. O filho 0 é o identificador da variável (rotulado `variavel`) e o nome do método fica em `valor_str`. Os métodos aceitos são `next`, `nextLine`, `nextInt`, `nextDouble`, `nextFloat`, `nextLong`, `nextShort`, `nextByte` e `nextBoolean`.

## Exemplos

Para `System.out.println("Ola");`:

```text
NO_PRINTLN
└── argumento: NO_EXPRESSAO
    └── filho[0]: NO_LITERAL_STRING (Ola)
```

Para `n = leitor.nextInt();`:

```text
NO_ATRIBUICAO
├── filho[0]: NO_IDENTIFICADOR (n)
└── filho[1]: NO_EXPRESSAO
    └── filho[0]: NO_LEITURA_SCANNER (nextInt)
        └── variavel: NO_IDENTIFICADOR (leitor)
```

## Decisões técnicas

- **Uma única regra para todas as leituras:** `metodo_scanner` devolve o nome do método como texto, então os nove métodos usam o mesmo nó `NO_LEITURA_SCANNER`. Adicionar um método novo exige só uma alternativa em `metodo_scanner`.
- **Método em `valor_str`:** o nó já tem o identificador como filho e a união `valor` estava livre. O nome é alocado com `strdup` e liberado em `liberarAST`.
- **Renomeação:** `NO_SCANNER` passou a se chamar `NO_INSTANCIACAO_SCANNER`, para diferenciar a instanciação da leitura. A ordem do enum e a do array de nomes em `ast.c` precisam continuar iguais.
- **Literal de string:** o lexer devolve `TOKEN_STRING_LITERAL` com o conteúdo sem as aspas, e a gramática cria `NO_LITERAL_STRING`.
- **`println()` sem argumento:** o nó não recebe filho (não é guardado um filho `NULL`).

## Dependência e stub (pedrorfb e Vinícius)

A construção completa das expressões é responsabilidade da issue de expressões. Enquanto ela não for integrada, `expressao_sintatica` é um stub. Todo caso devolve um ponteiro definido, e somente literais de string, acessos a array, leituras do Scanner e instanciações chegam à AST. Operadores, identificadores, números e booleanos devolvem `NULL`, então `NO_EXPRESSAO` fica sem filhos nesses casos. Operandos de operadores são descartados, de modo que `a + leitor.nextInt()` não mostra a leitura na AST.

Declarações com inicialização (`Scanner leitor = new Scanner(System.in);`) ainda não constroem filhos em `NO_VAR_DECL`, então o nó de instanciação só aparece na AST em atribuições (`leitor = new Scanner(System.in);`).

## Testes

Os casos ficam em `tests/ast/` e são executados por `tests/ast/run-tests.sh`, chamado por `make test-ast`:

- `valido_io_print_expressao.java`
- `valido_io_leitura_scanner.java`
- `invalido_print_sem_parenteses.java` (rejeitado com erro na linha 3)
