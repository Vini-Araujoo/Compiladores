# Issue 5: Entrada/Saída Padrão (I/O), Instanciação e Manipulação de Arrays

## 1. Identificação
* **Título:** Implementação das Regras de I/O (`System.out`, `Scanner`), Instanciação (`new`) e Arrays
* **Responsável:** Pessoa 5
* **Módulo:** `parser/parser.y`
* **Branch Git Sugerida:** `feat/issue-5-io-arrays`
* **Branch Alvo do Pull Request:** `main`
* **Dependências Externas:** Nenhuma (desenvolvimento isolado via Stubs)

---

## 2. Fluxo de Trabalho Git e Pull Request
Para garantir que o trabalho seja paralelo e não gere conflitos na branch principal:

1. **Criar uma nova branch a partir da `main` atualizada:**
   ```bash
   git checkout main
   git pull origin main
   git checkout -b feat/issue-5-io-arrays
   ```
2. **Desenvolver e validar isoladamente:**
   - Implementar as regras e stubs em `parser/parser.y`.
   - Adicionar a documentação e os casos de teste em `tests/parser/issue_5/`.
   - Garantir que `make clean && make` compila sem conflitos no Bison.
3. **Commit e Push:**
   ```bash
   git add .
   git commit -m "feat(parser): implementa regras de io, instanciacao e arrays"
   git push -u origin feat/issue-5-io-arrays
   ```
4. **Abrir Pull Request (PR):**
   - Abrir o PR no GitHub/GitLab com **base na branch `main`**.
   - **Nunca** comitar diretamente na `main`.
   - Descrever no PR o que foi implementado, anexando evidências da execução dos testes.

---

## 3. Contexto e Objetivo
Esta issue compreende os recursos especiais e bibliotecas padrão contemplados pelo subconjunto Java:
1. **Saída padrão:** Comandos `System.out.println(...)` e `System.out.print(...)`.
2. **Entrada padrão (`Scanner`):**
   - Instanciação do leitor: `new Scanner(System.in)`.
   - Chamadas dos métodos de leitura (`sc.nextInt()`, `sc.nextLine()`, `sc.nextDouble()`, `sc.nextFloat()`, `sc.nextLong()`, `sc.nextShort()`, `sc.nextByte()`, `sc.nextBoolean()`, `sc.next()`).
3. **Instanciação com operador `new`:**
   - Criação de arrays: `new tipo[expressao]`.
   - Criação de instâncias: `new Scanner(System.in)`.
4. **Manipulação de Arrays:**
   - Indexação e acesso a elementos: `ID[expressao]` (tanto como valor à direita em expressões quanto como alvo de atribuição à esquerda: `vetor[i] = valor;`).
5. **Chamadas de métodos gerais:**
   - Invocação de métodos simples e encadeados por ponto (`objeto.metodo(argumentos)`).

---

## 4. Estratégia de Desacoplamento (Zero Dependência)

Esses comandos especiais se comportam ora como **comandos** (ex.: `System.out.println(x);`), ora como **expressões/fatores** (ex.: `sc.nextInt();` ou `arr[i]`).

Para que você **trabalhe e valide tudo isoladamente**, você usará **stubs mínimos** para `expressao` e `tipo_base`.

### 4.1 Contrato de Interface
* **Não-terminais que VOCÊ PRODUZ (exporta para a equipe):**
  * `comando_io` (plugado diretamente como opção de `comando` na Issue 3)
  * `comando_print`
  * `instanciacao`
  * `leitura_scanner`
  * `acesso_array` (pode ser usado como lvalue de atribuição na Issue 1 e como termo/expressão na Issue 2)
  * `chamada_metodo`
* **Não-terminais que VOCÊ CONSOME:**
  * `expressao` (argumentos passados para print/métodos e índices de array)
  * `tipo_base` (tipos primitivos usados na criação do array com `new`)

### 4.2 Bloco de Stubs para Desenvolvimento Isolado
Adicione os seguintes stubs no seu `parser/parser.y` local para compilar e testar sem depender de ninguém:

```bison
// ==========================================
// STUBS TEMPORÁRIOS (NÃO DEPENDER DE 1 E 2)
// ==========================================
expressao:
      ID
    | NUMERO
    | ID TOKEN_MAIS NUMERO
    ;

tipo_base:
      TOKEN_INT
    | TOKEN_DOUBLE
    | TOKEN_STRING_TIPO
    ;

// Ponto de entrada de teste isolado da Issue 5:
programa:
      lista_instrucoes_io
    ;

lista_instrucoes_io:
      /* vazio */
    | lista_instrucoes_io comando_io
    | lista_instrucoes_io atribuicao_array
    ;

atribuicao_array:
      acesso_array TOKEN_ATRIB expressao TOKEN_PTOVIRG
    | ID TOKEN_ATRIB instanciacao TOKEN_PTOVIRG
    ;
```

Com esses stubs, você pode testar saídas `System.out.println`, leituras de `Scanner` e arrays de forma completamente independente!

---

## 5. Tokens Envolvidos
Consulte `lexer/lexer.l` para referência:
* **Entrada e Saída:** `TOKEN_SYSTEM`, `TOKEN_OUT`, `TOKEN_IN`, `TOKEN_PRINTLN`, `TOKEN_PRINT`, `TOKEN_SCANNER`
* **Métodos Scanner:** `TOKEN_NEXT`, `TOKEN_NEXT_LINE`, `TOKEN_NEXT_INT`, `TOKEN_NEXT_DOUBLE`, `TOKEN_NEXT_FLOAT`, `TOKEN_NEXT_LONG`, `TOKEN_NEXT_SHORT`, `TOKEN_NEXT_BYTE`, `TOKEN_NEXT_BOOLEAN`
* **Operador New:** `TOKEN_NEW`
* **Acesso e Delimitadores:** `TOKEN_PONTO` (`.`), `TOKEN_ABRE_COLCHETE` (`[`), `TOKEN_FECHA_COLCHETE` (`]`), `TOKEN_ABRE_PAR` (`(`), `TOKEN_FECHA_PAR` (`)`), `TOKEN_PTOVIRG` (`;`), `TOKEN_VIRGULA` (`,`)

---

## 6. Especificação das Regras a Implementar

Você deve modelar no Bison as regras sintáticas que atendam aos seguintes requisitos:

1. **Comandos de Saída (`comando_print`, `comando_io`):**
   - `System.out.println(argumento_opcional);` onde o argumento pode ser vazio ou uma expressão.
   - `System.out.print(expressao);` com expressão obrigatória.
   - Terminação obrigatória por ponto e vírgula.

2. **Entrada de Dados e Métodos do Scanner (`leitura_scanner`, `metodo_scanner`):**
   - Reconhecimento dos métodos de leitura invocados em identificadores: `id.nextInt()`, `id.nextLine()`, `id.nextDouble()`, `id.nextFloat()`, `id.nextLong()`, `id.nextShort()`, `id.nextByte()`, `id.nextBoolean()`, `id.next()`.

3. **Instanciação com operador `new` (`instanciacao`):**
   - Instanciação do Scanner via `new Scanner(System.in)`.
   - Criação dinâmica de arrays via `new tipo_base[expressao]`.

4. **Manipulação de Arrays (`acesso_array`):**
   - Indexação de array via `ID[expressao]`.
   - Deve ser suportada tanto como destino de atribuição à esquerda (`arr[i] = valor;`) quanto como termo/expressão.

5. **Invocação de Métodos Gerais (`chamada_metodo`):**
   - Chamada direta de função/método: `ID(argumentos_opcionais)`.
   - Chamada qualificada com ponto: `ID.ID(argumentos_opcionais)`.
   - Argumentos separados por vírgula.

*(Nota: a implementação das produções Bison fica a cargo do responsável pela issue; a referência técnica consolidada está disponível em `rep.md`)*

---

## 7. Casos de Teste Obrigatórios

Crie a pasta de testes: `tests/parser/issue_5/`

### 7.1 Testes Positivos (Devem ser aceitos com código 0)
1. `valido_print.java`:
   ```java
   System.out.println(mensagem);
   System.out.println(100);
   System.out.println();
   System.out.print(total);
   ```
2. `valido_scanner.java`:
   ```java
   leitor = new Scanner(System.in);
   sc.nextInt();
   sc.nextLine();
   sc.nextDouble();
   ```
3. `valido_arrays.java`:
   ```java
   vetor = new int[10];
   vetor[0] = 5;
   vetor[indice] = total;
   ```
4. `valido_chamadas_metodos.java`:
   ```java
   calcular();
   somar(a, b);
   objeto.executar(1, 2);
   ```

### 7.2 Testes Negativos (Devem falhar com erro sintático)
1. `invalido_print_sem_fechar_parenteses.java`:
   ```java
   System.out.println(mensagem;
   ```
2. `invalido_scanner_sem_system_in.java`:
   ```java
   new Scanner();
   ```
3. `invalido_array_sem_indice.java`:
   ```java
   vetor[] = 10;
   ```
4. `invalido_new_array_sem_tamanho.java`:
   ```java
   vetor = new int[];
   ```

---

## 8. Requisitos de Aceite (Definition of Done)

Para que esta issue seja aprovada e mergeada:

- [ ] **Fluxo Git & Pull Request:**
  - [ ] Trabalho realizado na branch dedicada `feat/issue-5-io-arrays` criada a partir da `main`.
  - [ ] Pull Request (PR) aberto no repositório apontando para a branch `main`.
  - [ ] Descrição do PR preenchida com as regras implementadas e evidência dos testes executados.
- [ ] **Documentação:**
  - [ ] Arquivo `docs/grammar/io_arrays.md` detalhando:
    - Regras BNF de entrada com `Scanner`, saída com `System.out`, criação e acesso de `arrays`.
    - Especificação de como as chamadas com ponto (`.`) foram modeladas sem gerar ambiguidades com identificadores comuns.
- [ ] **Testes:**
  - [ ] Criação dos arquivos `.java` da seção 7 dentro de `tests/parser/issue_5/`.
  - [ ] Script ou comando automatizado executando os testes:
    - Verificação de sucesso (código `0`) para todos os comandos de I/O e arrays válidos.
    - Emissão de `Erro sintatico` e código != 0 para sintaxes inválidas.
- [ ] **Compilação e Qualidade:**
  - [ ] Zero conflitos Shift/Reduce no Bison.
  - [ ] Facilidade de conexão final com as demais regras (Issue 1, 2, 3 e 4).
