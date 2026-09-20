# Issue 2: Expressões (Aritméticas, Relacionais, Lógicas) e Precedência de Operadores

## 1. Identificação
* **Título:** Implementação das Regras de Expressões e Precedência de Operadores
* **Responsável:** Pessoa 2
* **Módulo:** `parser/parser.y`
* **Branch Git Sugerida:** `feat/issue-2-expressoes`
* **Branch Alvo do Pull Request:** `main`
* **Dependências Externas:** Nenhuma (módulo autocontido)

---

## 2. Fluxo de Trabalho Git e Pull Request
Para garantir que o trabalho seja paralelo e não gere conflitos na branch principal:

1. **Criar uma nova branch a partir da `main` atualizada:**
   ```bash
   git checkout main
   git pull origin main
   git checkout -b feat/issue-2-expressoes
   ```
2. **Desenvolver e validar isoladamente:**
   - Implementar as regras e precedências em `parser/parser.y`.
   - Adicionar a documentação e os casos de teste em `tests/parser/issue_2/`.
   - Garantir que `make clean && make` compila sem conflitos no Bison.
3. **Commit e Push:**
   ```bash
   git add .
   git commit -m "feat(parser): implementa regras de expressoes e precedencia"
   git push -u origin feat/issue-2-expressoes
   ```
4. **Abrir Pull Request (PR):**
   - Abrir o PR no GitHub/GitLab com **base na branch `main`**.
   - **Nunca** comitar diretamente na `main`.
   - Descrever no PR o que foi implementado, anexando evidências da execução dos testes.

---

## 3. Contexto e Objetivo
Esta issue compreende a especificação sintática completa das expressões da linguagem, abrangendo:
1. Expressões binárias aritméticas (`+`, `-`, `*`, `/`).
2. Expressões relacionais e de igualdade (`<`, `>`, `<=`, `>=`, `==`, `!=`).
3. Expressões unárias de negação lógica (`!`) e sinal negativo (`-a`).
4. Agrupamento por parênteses `( expressao )`.
5. Fatores primários (identificadores `ID` e literais numéricos `NUMERO`).
6. Configuração rigorosa de **precedência e associatividade de operadores** no Bison (`%left`, `%right`, `%nonassoc`), garantindo que não ocorram ambiguidades gramaticais nem conflitos Shift/Reduce nas operações matemáticas e lógicas.

---

## 4. Estratégia de Desacoplamento (Zero Dependência)

As expressões formam o núcleo de cálculo consumido por todas as outras partes do compilador. Como a sua regra trabalha diretamente com os tokens básicos já emitidos pelo lexer (`ID`, `NUMERO` e operadores), **você não depende de nenhuma outra pessoa para começar e finalizar o seu trabalho**.

### 4.1 Contrato de Interface
* **Não-terminais que VOCÊ PRODUZ (exporta para a equipe):**
  * `expressao` (será consumida pela Issue 1 em declarações/atribuições, pela Issue 3 em condições `if`/`while`/`for`, e pela Issue 5 em índices de array e chamadas de método).
* **Não-terminais que VOCÊ CONSOME:**
  * Nenhum não-terminal de terceiros! Todos os elementos são tokens léxicos.

### 4.2 Ponto de Teste Isolado no Bison
Para testar toda a sua gramática de expressões no arquivo `parser.y` sem precisar de classes, funções ou declarações de variáveis, defina a regra inicial de teste como:

```bison
// Ponto de entrada temporário para validar expressões isoladas
programa:
      lista_expressoes
    ;

lista_expressoes:
      /* vazio */
    | lista_expressoes expressao TOKEN_PTOVIRG
    ;
```

Dessa forma, qualquer arquivo contendo sequências de expressões terminadas em `;` (ex.: `(a + 2) * 5 > c == !flag;`) pode ser testado diretamente com `make parser`.

---

## 5. Tokens Envolvidos
Consulte `lexer/lexer.l` para referência:
* **Identificadores e Literais:** `ID`, `NUMERO`
* **Operadores Aritméticos:** `TOKEN_MAIS` (`+`), `TOKEN_MENOS` (`-`), `TOKEN_VEZES` (`*`), `TOKEN_DIV` (`/`)
* **Operadores Relacionais e Igualdade:** `TOKEN_IGUAL` (`==`), `TOKEN_DIFERENTE` (`!=`), `TOKEN_MENOR` (`<`), `TOKEN_MAIOR` (`>`), `TOKEN_MENOR_IGUAL` (`<=`), `TOKEN_MAIOR_IGUAL` (`>=`)
* **Operadores Lógicos Unários:** `TOKEN_NEGACAO` (`!`)
* **Delimitadores:** `TOKEN_ABRE_PAR` (`(`), `TOKEN_FECHA_PAR` (`)`), `TOKEN_PTOVIRG` (`;`)

---

## 6. Especificação das Regras a Implementar

Você deve modelar no Bison as regras sintáticas e declarações de operadores que atendam aos seguintes requisitos:

1. **Operações Aritméticas Binárias:**
   - Adição (`+`), subtração (`-`), multiplicação (`*`) e divisão (`/`).
   - Respeitar a precedência matemática convencional: `*` e `/` têm prioridade maior que `+` e `-`.
   - Associatividade à esquerda para todas as quatro operações binárias.

2. **Operações Relacionais e de Igualdade:**
   - Comparações de magnitude: `<`, `>`, `<=`, `>=`.
   - Comparações de igualdade: `==`, `!=`.
   - A precedência relacional deve ser menor que a aritmética, e igualdade deve ter precedência menor que relacionais.

3. **Operadores Unários:**
   - Negação lógica (`!`).
   - Menos unário de sinal negativo (ex.: `-x` ou `-5`), que deve ter prioridade máxima sobre operadores binários. O uso de uma diretiva como `%prec` para o menos unário é recomendado para evitar ambiguidades com a subtração binária.

4. **Agrupamento e Fatores Primários:**
   - Expressões entre parênteses `( expressao )` para forçar ordem de avaliação.
   - Literais numéricos (`NUMERO`) e identificadores (`ID`).

*(Nota: a implementação das produções Bison e das diretivas `%left`/`%right` fica a cargo do responsável pela issue; a referência técnica consolidada está disponível em `rep.md`)*

---

## 7. Casos de Teste Obrigatórios

Crie a pasta de testes: `tests/parser/issue_2/`

### 7.1 Testes Positivos (Devem ser aceitos com código 0)
1. `valido_aritmetica_basica.java`:
   ```java
   10 + 20;
   a - b;
   total * 4 / 2;
   ```
2. `valido_precedencia_e_parenteses.java`:
   ```java
   2 + 3 * 4;
   (2 + 3) * 4;
   (a + b) / (c - d);
   ```
3. `valido_relacionais_e_igualdade.java`:
   ```java
   a > b;
   contador <= 10;
   valor == limite;
   resposta != 0;
   ```
4. `valido_unarios_e_complexas.java`:
   ```java
   -x + 10;
   !ativo;
   !(x > 5);
   (a + 2) * b >= c == !flag;
   ```

### 7.2 Testes Negativos (Devem falhar com erro sintático)
1. `invalido_operador_duplicado.java`:
   ```java
   a + * b;
   ```
2. `invalido_parenteses_abertos.java`:
   ```java
   (a + b * 2;
   ```
3. `invalido_operador_sem_operando.java`:
   ```java
   a + ;
   ```
4. `invalido_expressao_vazia.java`:
   ```java
   ();
   ```

---

## 8. Requisitos de Aceite (Definition of Done)

Para que esta issue seja aprovada e mergeada:

- [ ] **Fluxo Git & Pull Request:**
  - [ ] Trabalho realizado na branch dedicada `feat/issue-2-expressoes` criada a partir da `main`.
  - [ ] Pull Request (PR) aberto no repositório apontando para a branch `main`.
  - [ ] Descrição do PR preenchida com as regras implementadas e evidência dos testes executados.
- [ ] **Documentação:**
  - [ ] Arquivo `docs/grammar/expressoes.md` detalhando:
    - A tabela de precedência e associatividade dos operadores (do menor para o maior).
    - Explicação de como as ambiguidades gramaticais de operadores binários e do menos unário (`%prec UMINUS`) foram resolvidas.
    - Gramática formal BNF de expressões.
- [ ] **Testes:**
  - [ ] Criação dos arquivos `.java` da seção 7 dentro de `tests/parser/issue_2/`.
  - [ ] Script ou comando automatizado que valida que:
    - Todas as expressões válidas são aceitas sem erros sintáticos (código `0`).
    - Todas as expressões inválidas disparam `Erro sintatico` e encerram com código != 0.
- [ ] **Compilação e Qualidade:**
  - [ ] A execução do Bison (`bison -d -Wall parser/parser.y`) não pode apresentar nenhum conflito Shift/Reduce ou Reduce/Reduce.
  - [ ] Nenhuma dependência não-resolvida na compilação do executável.
