# Compiladores

Este projeto implementa um conversor de um subconjunto da linguagem Java para C procedural, sem o modelo de orientação a objetos. A proposta é receber um programa escrito na sintaxe definida para o subconjunto de Java e, nas etapas seguintes, gerar um programa equivalente em C.

O projeto ainda está em desenvolvimento. Nesta etapa foram implementadas as fases léxica e sintática: o Flex identifica tokens e o Bison verifica se a entrada segue a gramática do projeto. A tradução efetiva para C, assim como análise semântica, verificação de tipos e geração de código, serão implementadas nas próximas etapas.

Atualmente, a gramática reconhece a estrutura de um programa com uma classe de nível superior, atributos, métodos, parâmetros, blocos e comandos de controle de fluxo (`if`, `else`, `for`, `while`, `do while`, `switch`, `break` e `return`), expressões aritméticas/lógicas e instruções de entrada/saída e arrays.

---

## Organização do Projeto

- `lexer/`: Regras do analisador léxico escritas em Flex (`lexer/lexer.l`).
- `parser/`: Gramática sintática escrita em Bison (`parser/parser.y`).
- `docs/`: Documentação técnica do projeto:
  - [`docs/grammar/`](docs/grammar/): Especificação sintática (BNF) das 5 issues integradas:
    - [Estrutura do Programa e Classes](docs/grammar/estrutura_programa.md) (Issue 4)
    - [Declarações e Tipos](docs/grammar/declaracoes.md) (Issue 1)
    - [Expressões e Operadores](docs/grammar/expressoes.md) (Issue 2)
    - [Controle de Fluxo e Blocos](docs/grammar/controle_fluxo.md) (Issue 3)
    - [Entrada, Saída e Arrays](docs/grammar/io_arrays.md) (Issue 5)
  - [Erros Conhecidos e Limitações](docs/erros-conhecidos.md): Comportamentos e casos de borda mapeados.
- `tests/`: Casos de teste organizados por fase e por issue:
  - `tests/parser/`: Casos de teste sintáticos (`issue_1` a `issue_5`) e suíte automatizada (`run-tests.sh`).

---

## Como Rodar

Os comandos abaixo devem ser executados a partir da raiz do projeto:

### 1. Compilar o parser

Gera os analisadores com Flex e Bison e compila o binário `build/parser`:

```bash
make
# ou: make parser
```

Para limpar os arquivos gerados e o executável:

```bash
make clean
```

### 2. Executar manualmente com uma entrada Java

Para testar um arquivo Java:

```bash
./build/parser < Main.java
```

Para testar via linha de comando (inline):

```bash
printf 'class Exemplo { void executar() {} }\n' | ./build/parser
```

Ou executar de forma interativa (finalize a entrada com `Ctrl+D`):

```bash
./build/parser
```

---

## Como Executar os Testes

O projeto conta com uma suíte de testes sintáticos que valida tanto arquivos válidos (`valido_*.java`) quanto inválidos (`invalido_*.java`).

### Executar todos os testes (Issues 1 a 5)

```bash
make test
# ou diretamente via script:
bash tests/parser/run-tests.sh
```

### Executar testes de uma issue específica

Você pode informar o número da issue (1 a 5) ou o nome da pasta:

```bash
# Via make:
make test ISSUE=4
make test ISSUE=1

# Diretamente via script:
bash tests/parser/run-tests.sh 4
bash tests/parser/run-tests.sh issue_1
```

---

## Documentação Adicional

- [Documentação da Gramática Sintática](docs/grammar/)
- [Erros Conhecidos e Limitações](docs/erros-conhecidos.md)
- [Documentação do Analisador Léxico](tests/lexer/README.md)
