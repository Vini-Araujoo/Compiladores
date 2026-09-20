# Issue 1: Tipos, Declaração e Inicialização de Variáveis

## 1. Identificação

- **Título:** Implementação das Regras de Tipos, Declarações e Atribuições
- **Responsável:** Pessoa 1
- **Módulo:** `parser/parser.y`
- **Branch Git Sugerida:** `feat/issue-1-declaracoes`
- **Branch Alvo do Pull Request:** `main`
- **Dependências Externas:** Nenhuma (desenvolvimento isolado via Stubs)

---

## 2. Fluxo de Trabalho Git e Pull Request

Para garantir que o trabalho seja paralelo e não gere conflitos na branch principal:

1. **Criar uma nova branch a partir da `main` atualizada:**
   ```bash
   git checkout main
   git pull origin main
   git checkout -b feat/issue-1-declaracoes
   ```
2. **Desenvolver e validar isoladamente:**
   - Implementar as regras e stubs em `parser/parser.y`.
   - Adicionar a documentação e os casos de teste em `tests/parser/issue_1/`.
   - Garantir que `make clean && make` compila sem erros.
3. **Commit e Push:**
   ```bash
   git add .
   git commit -m "feat(parser): implementa regras de declaracoes e atribuicoes"
   git push -u origin feat/issue-1-declaracoes
   ```
4. **Abrir Pull Request (PR):**
   - Abrir o PR no GitHub com **base na branch `main`**.
   - **Nunca** comitar diretamente na `main`.
   - Descrever no PR o que foi implementado, anexando evidências da execução dos testes.

---

## 3. Contexto e Objetivo

Esta issue compreende a especificação sintática de:

1. Especificadores de tipos primitivos (`byte`, `short`, `int`, `long`, `boolean`, `char`, `float`, `double`) e tipo `String`.
2. Modificador `final` para declaração de constantes locais.
3. Declaração de variáveis simples e múltiplas (`int a;`, `int a, b;`, `int a = 10;`, `int a = 1, b = 2;`).
4. Comandos de atribuição simples e composta (`=`, `+=`, `-=`).
5. Comandos de incremento e decremento (`++`, `--`) pós e pré-fixados.

O objetivo é permitir que o compilador reconheça qualquer instrução de definição de variáveis e manipulação de valores no subconjunto da linguagem Java.

---

## 4. Estratégia de Desacoplamento (Zero Dependência)

Para que você **não precise esperar** a implementação das expressões (Issue 2) ou dos blocos/estruturas de controle (Issue 3), você utilizará um **Stub de Expressão** no seu ambiente local de desenvolvimento.

### 4.1 Contrato de Interface

- **Não-terminais que VOCÊ PRODUZ (exporta para a equipe):**
  - `declaracao_variavel`
  - `atribuicao`
  - `tipo`
  - `tipo_primitivo`
  - `modificador_variavel`
- **Não-terminais que VOCÊ CONSOME:**
  - `expressao` (usada no valor inicial da declaração e no lado direito da atribuição).

### 4.2 Bloco de Stubs para Desenvolvimento Isolado

Durante o desenvolvimento isolado da sua branch, utilize o seguinte stub em seu arquivo de gramática para compilar e testar sem depender de outros membros:

```bison
// ==========================================
// STUB TEMPORÁRIO (NÃO DEPENDER DA ISSUE 2)
// Substitui a gramática completa de expressões
// ==========================================
expressao:
      ID
    | NUMERO
    | ID TOKEN_MAIS NUMERO
    ;

// Regra inicial de teste isolado da Issue 1:
programa:
      lista_instrucoes
    ;

lista_instrucoes:
      /* vazio */
    | lista_instrucoes declaracao_variavel
    | lista_instrucoes atribuicao TOKEN_PTOVIRG
    ;
```

Com esse stub, você valida declarações como `int x = 10;` ou `int x = y + 2;` imediatamente com `make parser` sem que a Issue 2 precise estar pronta!

---

## 5. Tokens Envolvidos

Consulte `lexer/lexer.l` para referência:

- **Tipos:** `TOKEN_BYTE`, `TOKEN_SHORT`, `TOKEN_INT`, `TOKEN_LONG`, `TOKEN_BOOLEAN`, `TOKEN_CHAR`, `TOKEN_FLOAT`, `TOKEN_DOUBLE`, `TOKEN_STRING_TIPO`
- **Modificadores:** `TOKEN_FINAL`
- **Identificadores e Literais:** `ID`, `NUMERO`
- **Operadores de Atribuição e Unários:** `TOKEN_ATRIB` (`=`), `TOKEN_SOMA_ATRIB` (`+=`), `TOKEN_SUB_ATRIB` (`-=`), `TOKEN_INCREMENTO` (`++`), `TOKEN_DECREMENTO` (`--`)
- **Delimitadores:** `TOKEN_PTOVIRG` (`;`), `TOKEN_VIRGULA` (`,`)

---

## 6. Especificação das Regras a Implementar

Você deve modelar no Bison as regras sintáticas que atendam aos seguintes requisitos:

1. **Tipos de Dados (`tipo`, `tipo_primitivo`):**
   - Suporte a todos os tipos primitivos: `byte`, `short`, `int`, `long`, `boolean`, `char`, `float`, `double`.
   - Suporte ao tipo `String` (`TOKEN_STRING_TIPO`).

2. **Modificadores de Variável:**
   - Suporte ao modificador `final` precedendo o tipo da declaração.

3. **Declarações de Variáveis (`declaracao_variavel`):**
   - Declaração simples com ou sem modificador `final`.
   - Declaração com inicialização opcional via `= expressao`.
   - Declaração de múltiplas variáveis separadas por vírgula na mesma instrução (ex.: `int a, b = 2, c;`).
   - Todas as declarações devem ser finalizadas obrigatoriamente com ponto e vírgula (`;`).

4. **Comandos de Atribuição (`atribuicao`):**
   - Atribuição simples (`= expressao`).
   - Atribuições compostas de adição e subtração (`+= expressao` e `-= expressao`).
   - Incremento e decremento tanto pós-fixados (`x++`, `x--`) quanto pré-fixados (`++x`, `--x`).

*(Nota: a implementação das produções Bison fica a cargo do responsável pela issue; a referência técnica consolidada está disponível em `rep.md`)*

---

## 7. Casos de Teste Obrigatórios

Crie a pasta de testes: `tests/parser/issue_1/`

### 7.1 Testes Positivos (Devem ser aceitos com código 0)

1. `valido_primitivos.java`:
   ```java
   int a;
   double salario;
   boolean flag;
   char letra;
   String nome;
   ```
2. `valido_inicializacoes.java`:
   ```java
   int contador = 42;
   double preco = 10;
   final int LIMITE = 100;
   ```
3. `valido_multiplas_declaracoes.java`:
   ```java
   int x, y, z;
   int a = 1, b, c = 3;
   ```
4. `valido_atribuicoes.java`:
   ```java
   x = 5;
   contador += 1;
   saldo -= 10;
   x++;
   y--;
   ++x;
   --y;
   ```

### 7.2 Testes Negativos (Devem falhar com erro sintático)

1. `invalido_sem_ponto_virgula.java`:
   ```java
   int x = 10
   ```
2. `invalido_tipo_inexistente.java`:
   ```java
   tipoInvalido x = 10;
   ```
3. `invalido_atribuicao_sem_expressao.java`:
   ```java
   x = ;
   ```
4. `invalido_virgula_sobrando.java`:
   ```java
   int a, ;
   ```

---

## 8. Requisitos de Aceite (Definition of Done)

Para que esta issue seja aprovada e mergeada:

- [ ] **Fluxo Git & Pull Request:**
  - [ ] Trabalho realizado na branch dedicada `feat/issue-1-declaracoes` criada a partir da `main`.
  - [ ] Pull Request (PR) aberto no repositório apontando para a branch `main`.
  - [ ] Descrição do PR preenchida com as regras implementadas e evidência dos testes executados.
- [ ] **Documentação:**
  - [ ] Arquivo `docs/grammar/declaracoes.md` (ou seção dedicada no README) documentando em formato BNF todas as regras de tipos, declaração de variáveis e atribuições.
  - [ ] Explicação de como as declarações múltiplas (`int a, b = 2;`) e constantes `final` foram modeladas.
  - [ ] Registro dos stubs utilizados para testar antes da integração final.
- [ ] **Testes:**
  - [ ] Criação dos arquivos `.java` da seção 7 dentro de `tests/parser/issue_1/`.
  - [ ] Script ou target no Makefile que executa os testes contra o parser compilado:
    - Todos os casos positivos devem retornar código `0` e imprimir sucesso.
    - Todos os casos negativos devem retornar código diferente de zero com mensagem de erro sintático.
- [ ] **Compilação e Qualidade:**
  - [ ] `bison -d parser/parser.y` não deve introduzir novos conflitos Shift/Reduce ou Reduce/Reduce.
  - [ ] O código deve compilar sem warnings adicionais no `gcc`.
