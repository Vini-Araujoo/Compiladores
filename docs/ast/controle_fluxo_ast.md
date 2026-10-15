# AST de Controle de Fluxo e Laços

Este documento descreve a representação dos laços `while`, `do-while` e `for` na AST (Issue 7.2).

## Nós de repetição

| Nó | Filho 0 | Filho 1 |
| --- | --- | --- |
| `NO_WHILE` | Condição (`NO_CONDICAO`) | Bloco (`NO_BLOCO`) |
| `NO_DO_WHILE` | Bloco (`NO_BLOCO`) | Condição (`NO_CONDICAO`) |

A ordem representa a ordem de execução e a forma sintática: `while` testa antes do bloco; `do-while` executa o bloco antes do teste. A condição é o nó produzido pela regra `condicao`; a construção interna da expressão pertence à etapa responsável por expressões.

## Nó `NO_FOR`

`NO_FOR` possui quatro posições fixas:

| Índice | Segmento | Exemplos |
| --- | --- | --- |
| `filhos[0]` | Inicialização | `i = 0` ou declaração local |
| `filhos[1]` | Teste da condição | `i < limite` |
| `filhos[2]` | Incremento | `i++`, `++i`, `i--`, `--i` ou atribuição |
| `filhos[3]` | Bloco | Corpo do laço (`NO_BLOCO`) |

Cláusulas omitidas são representadas por ponteiros `NULL` na posição correspondente. Por exemplo, `for (;;) {}` tem `NULL` nos filhos 0, 1 e 2, e o bloco no filho 3. Os `NULL` devem ser preservados no vetor para que os índices não mudem.

`adicionarFilho` aceita um pai não nulo e armazena o ponteiro do filho mesmo quando ele é `NULL`. A liberação recursiva já ignora ponteiros nulos. Qualquer rotina que percorra ou imprima a AST deve verificar cada ponteiro antes de acessar seus campos.

## Impressão esperada

Quando a rotina de impressão da AST estiver disponível, ela deve rotular os segmentos do `for` em vez de mostrar apenas uma sequência sem nomes. Para cláusulas ausentes, deve indicar que o segmento está vazio. Exemplo:

```text
NO_FOR
├── inicializacao: <vazio>
├── condicao: <vazio>
├── incremento: <vazio>
└── bloco: NO_BLOCO
```

## Testes

Os casos de parser ficam em `tests/ast/` e são executados pelo script `tests/ast/run-tests.sh`, chamado por `make test-ast`:

- `valido_laco_while.java`
- `valido_laco_do_while.java`
- `valido_laco_for_completo.java`
- `valido_laco_for_vazio.java`
- `invalido_do_while_sem_ponto_virgula.java`

Os casos positivos verificam aceitação sintática; o caso negativo verifica a rejeição do `do-while` sem `;` e a linha informada. Esses testes não inspecionam a estrutura dos nós da AST.
