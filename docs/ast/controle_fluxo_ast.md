# AST de Controle de Fluxo, Laços e Seleção

Este documento descreve a representação dos laços `while`, `do-while` e `for` (Issue 7.2), além da seleção `switch` (Issue 7.3).

## Nós de repetição

| Nó | Filho 0 | Filho 1 |
| --- | --- | --- |
| `NO_WHILE` | Condição (`NO_CONDICAO`) | Bloco (`NO_BLOCO`) |
| `NO_DO_WHILE` | Bloco (`NO_BLOCO`) | Condição (`NO_CONDICAO`) |

A ordem representa a ordem de execução e a forma sintática: `while` testa antes do bloco; `do-while` executa o bloco antes do teste. A condição é o nó produzido pela regra `condicao`; a construção interna da expressão pertence à etapa responsável por expressões.

Enquanto essa etapa não estiver integrada, a regra `expressao` retorna um nó temporário `NO_EXPRESSAO` sem filhos. Ele permite que regras que dependem de uma expressão tenham um valor AST definido, mas não representa os operadores nem os operandos da expressão.

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

## Nós de seleção (`switch`)

`NO_SWITCH` possui três posições: `filhos[0]` é a expressão seletora, `filhos[1]` é o nó `NO_CASES`, e `filhos[2]` é `NO_DEFAULT` ou `NULL` quando a cláusula padrão não existe. `NO_CASES` preserva a ordem dos ramos e contém um `NO_CASE` por cláusula `case`.

Cada `NO_CASE` tem a expressão do rótulo em `filhos[0]` e o bloco de comandos associado em `filhos[1]`. `NO_DEFAULT`, quando presente, tem o bloco padrão em `filhos[0]`. `break;` é representado por um nó terminal `NO_BREAK`, sem filhos.

Exemplo estrutural para `switch (2) { case 1: break; default: break; }`:

```text
NO_SWITCH
├── seletor: NO_EXPRESSAO
├── casos: NO_CASES
│   └── case: NO_CASE
│       ├── valor: NO_EXPRESSAO
│       └── comandos: NO_BLOCO
│           └── NO_BREAK
└── default: NO_DEFAULT
    └── comandos: NO_BLOCO
        └── NO_BREAK
```

A regra de expressão ainda é um stub: ela valida a sintaxe, mas cria apenas `NO_EXPRESSAO`, sem guardar o literal nem os operandos. Assim, a AST já permite validar os nós e sua organização; a expressão seletora e os valores dos `case` não são impressos semanticamente ainda.

## Impressão da AST

`imprimirAST` percorre os nós e seus filhos, indica posições vazias e rotula os segmentos de `NO_FOR`, `NO_SWITCH`, `NO_CASE` e `NO_DEFAULT`. O executável chama essa rotina depois de uma análise sintática bem-sucedida, antes da mensagem de sucesso. A impressão usa os nós que as regras já conectaram à árvore; regras que ainda são stubs podem resultar em subárvores incompletas.

## Testes de `switch`

`make test-ast` também executa os casos de `tests/ast/`:

- `valido_switch_com_cases_e_default.java`: exige a impressão de `NO_SWITCH`, lista e ramos, `NO_DEFAULT` e `NO_BREAK`.
- `valido_switch_sem_default.java`: exige que a posição de `default` apareça vazia.
- `invalido_switch_sem_chaves.java`: verifica rejeição sintática com número da linha.
