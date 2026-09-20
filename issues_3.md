# Issue 3: Estruturas de Controle de Fluxo, Desvios e Blocos

## 1. Identificação
* **Título:** Implementação das Regras de Controle de Fluxo (Condicionais, Repetições, Saltos e Blocos)
* **Responsável:** Pessoa 3
* **Módulo:** `parser/parser.y`
* **Branch Git Sugerida:** `feat/issue-3-controle-fluxo`
* **Branch Alvo do Pull Request:** `main`
* **Dependências Externas:** Nenhuma (desenvolvimento isolado via Stubs)

---

## 2. Fluxo de Trabalho Git e Pull Request
Para garantir que o trabalho seja paralelo e não gere conflitos na branch principal:

1. **Criar uma nova branch a partir da `main` atualizada:**
   ```bash
   git checkout main
   git pull origin main
   git checkout -b feat/issue-3-controle-fluxo
   ```
2. **Desenvolver e validar isoladamente:**
   - Implementar as regras e stubs em `parser/parser.y`.
   - Adicionar a documentação e os casos de teste em `tests/parser/issue_3/`.
   - Garantir que `make clean && make` compila sem conflitos no Bison.
3. **Commit e Push:**
   ```bash
   git add .
   git commit -m "feat(parser): implementa regras de controle de fluxo e blocos"
   git push -u origin feat/issue-3-controle-fluxo
   ```
4. **Abrir Pull Request (PR):**
   - Abrir o PR no GitHub/GitLab com **base na branch `main`**.
   - **Nunca** comitar diretamente na `main`.
   - Descrever no PR o que foi implementado, anexando evidências da execução dos testes.

---

## 3. Contexto e Objetivo
Esta issue compreende a especificação sintática de todas as estruturas de controle de execução da linguagem:
1. Condicionais `if` e `if-else` (incluindo tratamento formal da ambiguidade do *dangling else*).
2. Seleção de casos com `switch`, `case`, `default` e `break`.
3. Laços de repetição `while`, `do-while` e `for (init; condicao; incremento)`.
4. Comandos de desvio e retorno (`break;`, `return;`, `return expressao;`).
5. Conceito fundamental de bloco de código `{ lista_comandos }` e comando simples terminado em `;`.

---

## 4. Estratégia de Desacoplamento (Zero Dependência)

As estruturas de controle dependem de expressões (para suas condições) e de declarações/atribuições (para os corpos e cabeçalhos do `for`). Para que você **trabalhe 100% de forma paralela e independente**, você usará **stubs mínimos** para `expressao`, `declaracao_variavel` e `atribuicao`.

### 4.1 Contrato de Interface
* **Não-terminais que VOCÊ PRODUZ (exporta para a equipe):**
  * `comando`
  * `lista_comandos`
  * `bloco` (será consumido pela Issue 4 no corpo dos métodos)
  * `comando_if`
  * `comando_switch`
  * `comando_while`
  * `comando_do_while`
  * `comando_for`
  * `comando_salto`
* **Não-terminais que VOCÊ CONSOME:**
  * `expressao` (condição do if, while, for, switch, valor de retorno)
  * `declaracao_variavel` (comandos permitidos dentro de blocos e no init do `for`)
  * `atribuicao` (comandos permitidos em blocos e no passo do `for`)

### 4.2 Bloco de Stubs para Desenvolvimento Isolado
Adicione os seguintes stubs no seu `parser/parser.y` durante o desenvolvimento e testes:

```bison
// ==========================================
// STUBS TEMPORÁRIOS (NÃO DEPENDER DE 1 E 2)
// ==========================================
expressao:
      ID
    | NUMERO
    ;

declaracao_variavel:
      TOKEN_INT ID TOKEN_PTOVIRG
    | TOKEN_INT ID TOKEN_ATRIB NUMERO TOKEN_PTOVIRG
    ;

atribuicao:
      ID TOKEN_ATRIB NUMERO
    | ID TOKEN_INCREMENTO
    ;

// Regra inicial de teste isolado da Issue 3:
programa:
      lista_comandos
    ;
```

Com esses stubs, você pode testar qualquer aninhamento de `if`, `while`, `for`, `switch` e `blocos` sem precisar que a Pessoa 1 ou Pessoa 2 tenham terminado suas tarefas!

---

## 5. Tokens Envolvidos
Consulte `lexer/lexer.l` para referência:
* **Condicionais e Seleção:** `TOKEN_IF`, `TOKEN_ELSE`, `TOKEN_SWITCH`, `TOKEN_CASE`, `TOKEN_DEFALUT`
* **Repetição:** `TOKEN_WHILE`, `TOKEN_DO`, `TOKEN_FOR`
* **Saltos:** `TOKEN_BREAK`, `TOKEN_RETURN`
* **Delimitadores e Operadores:** `TOKEN_ABRE_PAR` (`(`), `TOKEN_FECHA_PAR` (`)`), `TOKEN_ABRE_CHAVE` (`{`), `TOKEN_FECHA_CHAVE` (`}`), `TOKEN_DOIS_PONTOS` (`:`), `TOKEN_PTOVIRG` (`;`)

---

## 6. Especificação das Regras a Implementar

Você deve modelar no Bison as regras sintáticas que atendam aos seguintes requisitos:

1. **Blocos de Código (`bloco`):**
   - Sequência de zero ou mais comandos entre chaves `{ ... }`.

2. **Condicionais `if` e `if-else` (`comando_if`):**
   - Sintaxe com `if (expressao) comando`.
   - Sintaxe com `if (expressao) comando else comando`.
   - **Atenção ao Dangling Else:** Deve-se configurar a precedência no Bison (usando diretivas como `%nonassoc LOWER_THAN_ELSE` e `%nonassoc TOKEN_ELSE`) para garantir que qualquer `else` seja associado ao `if` mais próximo/interno, sem conflito Shift/Reduce.

3. **Seleção com `switch` (`comando_switch`):**
   - Cabeçalho `switch (expressao) { ... }`.
   - Corpo contendo zero ou mais cláusulas `case NUMERO:` e opcionalmente `default:`.
   - Cada cláusula `case` ou `default` pode conter múltiplos comandos.

4. **Laços de Repetição (`comando_while`, `comando_do_while`, `comando_for`):**
   - `while (expressao) comando`.
   - `do comando while (expressao);` (com terminação obrigatória por ponto e vírgula).
   - `for (inicializacao; condicao; incremento) comando`, onde cada uma das 3 partes do cabeçalho pode ser opcional/vazia.

5. **Comandos de Salto e Retorno (`comando_salto`):**
   - `break;`.
   - `return;` (sem valor de retorno) e `return expressao;` (com retorno de valor).

6. **Comando Genérico (`comando`):**
   - Pode ser um bloco, uma declaração de variável, uma atribuição terminada em `;`, qualquer estrutura de controle, um comando de salto, ou um comando vazio `;`.

*(Nota: a implementação das produções Bison fica a cargo do responsável pela issue; a referência técnica consolidada está disponível em `rep.md`)*

---

## 7. Casos de Teste Obrigatórios

Crie a pasta de testes: `tests/parser/issue_3/`

### 7.1 Testes Positivos (Devem ser aceitos com código 0)
1. `valido_if_else.java`:
   ```java
   if (contador) {
       contador = 1;
   } else {
       contador = 2;
   }

   if (limite)
       contador = 0;
   ```
2. `valido_while_e_dowhile.java`:
   ```java
   while (condicao) {
       contador++;
       if (limite) {
           break;
       }
   }

   do {
       contador--;
   } while (condicao);
   ```
3. `valido_for.java`:
   ```java
   for (int i = 0; i; i++) {
       contador = 1;
   }
   for (;;) {
       break;
   }
   ```
4. `valido_switch.java`:
   ```java
   switch (opcao) {
       case 1:
           contador = 10;
           break;
       case 2:
           contador = 20;
           break;
       default:
           return 0;
   }
   ```
5. `valido_aninhamento_e_retorno.java`:
   ```java
   {
       int x = 10;
       if (x) {
           while (x) {
               return x;
           }
       }
       return;
   }
   ```

### 7.2 Testes Negativos (Devem falhar com erro sintático)
1. `invalido_if_sem_parenteses.java`:
   ```java
   if x { return; }
   ```
2. `invalido_while_sem_condicao.java`:
   ```java
   while () { }
   ```
3. `invalido_do_while_sem_ponto_virgula.java`:
   ```java
   do { } while (x)
   ```
4. `invalido_for_sem_delimitadores.java`:
   ```java
   for (i = 0 i < 10 i++) { }
   ```

---

## 8. Requisitos de Aceite (Definition of Done)

Para que esta issue seja aprovada e mergeada:

- [ ] **Fluxo Git & Pull Request:**
  - [ ] Trabalho realizado na branch dedicada `feat/issue-3-controle-fluxo` criada a partir da `main`.
  - [ ] Pull Request (PR) aberto no repositório apontando para a branch `main`.
  - [ ] Descrição do PR preenchida com as regras implementadas e evidência dos testes executados.
- [ ] **Documentação:**
  - [ ] Arquivo `docs/grammar/controle_fluxo.md` detalhando:
    - As regras BNF de todas as instruções condicionais, laços, switches e blocos.
    - A justificativa e mecanismo de resolução da ambiguidade do dangling else (`%nonassoc LOWER_THAN_ELSE` e `%nonassoc TOKEN_ELSE`).
    - Como comandos isolados vs blocos foram tratados.
- [ ] **Testes:**
  - [ ] Criação dos arquivos `.java` da seção 7 dentro de `tests/parser/issue_3/`.
  - [ ] Script ou comando automatizado executando os testes:
    - Validação de que todas as estruturas válidas retornam código `0`.
    - Validação de que as estruturas malformadas emitem `Erro sintatico` e encerram com código != 0.
- [ ] **Compilação e Qualidade:**
  - [ ] Bison executado sem conflitos inesperados.
  - [ ] Regras limpas prontas para se conectarem com a Issue 4 (que consome `bloco`) e as Issues 1, 2 e 5.
