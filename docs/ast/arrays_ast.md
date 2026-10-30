# AST de Arrays

Este documento descreve a representação de arrays unidimensionais na AST: a instanciação (`new tipo[tamanho]`) e o acesso por índice (`vetor[indice]`).

## Nós

| Nó | Filho 0 | Filho 1 | Campo `tipo_dado` |
| --- | --- | --- | --- |
| `NO_INSTANCIACAO_ARRAY` | Expressão de tamanho | (não possui) | Tipo base do array (`TIPO_INT`, `TIPO_DOUBLE` ou `TIPO_STRING`) |
| `NO_ACESSO_ARRAY` | Identificador do array (`NO_IDENTIFICADOR`) | Expressão do índice | (não utilizado) |

Os dois nós registram a linha do código-fonte em `linha`.

### `NO_INSTANCIACAO_ARRAY`

Produzido pela regra `instanciacao` para `new tipo[expressao]`. O tipo base é guardado em `tipo_dado` e a expressão de tamanho é o único filho. O tamanho pode ser qualquer expressão, inclusive dinâmica.

### `NO_ACESSO_ARRAY`

Produzido pela regra `acesso_array` para `identificador[expressao]`. O filho 0 é o nome do array e o filho 1 é a expressão do índice, que também pode ser dinâmica.

## R-value e L-value

O mesmo nó `NO_ACESSO_ARRAY` serve nos dois papéis, e a diferença está só na posição dele na árvore:

- **L-value:** na atribuição `vetor[i] = expr;`, o acesso é o filho 0 de `NO_ATRIBUICAO` e a expressão atribuída é o filho 1.
- **R-value:** em uma expressão (`x = vetor[i];`), o acesso é um operando dela.

## Exemplo

Para `vetor[0] = 5;`:

```text
NO_ATRIBUICAO
├── filho[0]: NO_ACESSO_ARRAY
│   ├── array: NO_IDENTIFICADOR (vetor)
│   └── indice: NO_EXPRESSAO
└── filho[1]: NO_EXPRESSAO
```

## Decisões técnicas

- **Tipo base em `tipo_dado`:** o nó só tem um campo de tipo, então ele guarda o tipo base (o elemento) e não `TIPO_ARRAY`. O fato de ser um array é indicado pelo próprio tipo do nó.
- **Novo nó em vez de reaproveitar `NO_NOVA_INSTANCIA`:** `NO_INSTANCIACAO_ARRAY` separa a instanciação de arrays de outras instanciações (como `new Scanner(System.in)`). `NO_NOVA_INSTANCIA` foi mantido no enum para não quebrar código existente.
- **Rótulos na impressão:** `rotuloFilho` mostra `tamanho` no filho de `NO_INSTANCIACAO_ARRAY` e `array` / `indice` nos filhos de `NO_ACESSO_ARRAY`.
- **Arrays unidimensionais:** não há suporte a arrays multidimensionais.

## Dependência e stub (Issue 9.1)

A construção das expressões é responsabilidade da Issue 9.1 (pedrorfb). Enquanto ela não for integrada, a regra `expressao` devolve um nó temporário `NO_EXPRESSAO` sem filhos. Ele é usado como índice e como tamanho, o que basta para validar a estrutura dos nós.

Por causa disso, dentro de `expressao_sintatica` os nós `NO_ACESSO_ARRAY` e `NO_INSTANCIACAO_ARRAY` ainda não são propagados para a árvore final. O uso como L-value já aparece na AST. A integração completa do uso como R-value e da instanciação depende da 9.1.

## Testes

Os casos ficam em `tests/ast/` e são executados por `tests/ast/run-tests.sh`, chamado por `make test-ast`:

- `valido_alocacao_array.java`
- `valido_atribuicao_em_indice_array.java`
- `invalido_array_sem_tamanho.java`

Os testes usam apenas literais inteiros (`new int[10]`, `vetor[0]`). Os positivos verificam a aceitação sintática (código de saída 0) e o negativo verifica a rejeição de `new int[]` com mensagem de erro e linha.