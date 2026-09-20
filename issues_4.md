# Issue 4: Estrutura Global do Programa, Declaração de Classes e Métodos

## 1. Identificação
* **Título:** Implementação das Regras de Programa, Classes, Métodos e Modificadores
* **Responsável:** Pessoa 4
* **Módulo:** `parser/parser.y`
* **Branch Git Sugerida:** `feat/issue-4-estrutura-programa`
* **Branch Alvo do Pull Request:** `main`
* **Dependências Externas:** Nenhuma (desenvolvimento isolado via Stubs)

---

## 2. Fluxo de Trabalho Git e Pull Request
Para garantir que o trabalho seja paralelo e não gere conflitos na branch principal:

1. **Criar uma nova branch a partir da `main` atualizada:**
   ```bash
   git checkout main
   git pull origin main
   git checkout -b feat/issue-4-estrutura-programa
   ```
2. **Desenvolver e validar isoladamente:**
   - Implementar as regras e stubs em `parser/parser.y`.
   - Adicionar a documentação e os casos de teste em `tests/parser/issue_4/`.
   - Garantir que `make clean && make` compila sem conflitos no Bison.
3. **Commit e Push:**
   ```bash
   git add .
   git commit -m "feat(parser): implementa regras de classes, metodos e modificadores"
   git push -u origin feat/issue-4-estrutura-programa
   ```
4. **Abrir Pull Request (PR):**
   - Abrir o PR no GitHub/GitLab com **base na branch `main`**.
   - **Nunca** comitar diretamente na `main`.
   - Descrever no PR o que foi implementado, anexando evidências da execução dos testes.

---

## 3. Contexto e Objetivo
Esta issue compreende a especificação da estrutura global (top-level) de um programa em Java:
1. Ponto de entrada da gramática (`%start programa`), composto por uma ou mais declarações de classes.
2. Declaração de classes com modificadores (`public`, `final`, etc.) e corpo entre chaves.
3. Modificadores de acesso e escopo (`public`, `private`, `protected`, `static`, `final`).
4. Membros de classe: atributos (campos) e declaração de métodos.
5. Assinatura formal de métodos: tipo de retorno (`tipo` ou `void`), identificador, lista de parâmetros formais (incluindo parâmetros de array, como `String[] args`), e o corpo do método (`bloco`).

---

## 4. Estratégia de Desacoplamento (Zero Dependência)

A estrutura de classes e métodos é a "casca externa" do programa. O corpo do método é um `bloco` (produzido pela Issue 3), os tipos de parâmetros/retornos são `tipo` (produzido pela Issue 1), e os atributos da classe são `declaracao_variavel` (Issue 1).

Para você **trabalhar e testar de forma 100% independente**, basta declarar **stubs mínimos** para `bloco`, `tipo` e `declaracao_variavel`.

### 4.1 Contrato de Interface
* **Não-terminais que VOCÊ PRODUZ (exporta para a equipe):**
  * `programa` (símbolo inicial `%start programa` que une todo o compilador!)
  * `declaracao_classe`
  * `corpo_classe`
  * `membro_classe`
  * `declaracao_metodo`
  * `lista_parametros`
  * `parametro`
  * `modificadores`
  * `modificador`
  * `tipo_retorno`
* **Não-terminais que VOCÊ CONSOME:**
  * `bloco` (corpo executável dos métodos)
  * `tipo` (tipo dos parâmetros e do retorno quando não for void)
  * `declaracao_variavel` (atributos definidos dentro da classe)

### 4.2 Bloco de Stubs para Desenvolvimento Isolado
Adicione os seguintes stubs no seu `parser/parser.y` para compilar e testar isoladamente:

```bison
// ==========================================
// STUBS TEMPORÁRIOS (NÃO DEPENDER DE 1 E 3)
// ==========================================
bloco:
      TOKEN_ABRE_CHAVE TOKEN_FECHA_CHAVE
    ;

tipo:
      TOKEN_INT
    | TOKEN_DOUBLE
    | TOKEN_STRING_TIPO
    ;

declaracao_variavel:
      tipo ID TOKEN_PTOVIRG
    | tipo ID TOKEN_ATRIB NUMERO TOKEN_PTOVIRG
    ;
```

Com esses stubs de poucas linhas, você valida classes inteiras com métodos, variáveis de instância, método `main` e listas de parâmetros sem depender de mais ninguém!

---

## 5. Tokens Envolvidos
Consulte `lexer/lexer.l` para referência:
* **Classes:** `TOKEN_CLASS`
* **Modificadores:** `TOKEN_PUBLIC`, `TOKEN_PRIVATE`, `TOKEN_PROTECTED`, `TOKEN_STATIC`, `TOKEN_FINAL`
* **Retorno Vazio:** `TOKEN_VOID`
* **Identificadores:** `ID`
* **Tipos de Array nos Parâmetros (ex: `String[] args`):** `TOKEN_ABRE_COLCHETE`, `TOKEN_FECHA_COLCHETE`
* **Delimitadores:** `TOKEN_ABRE_CHAVE` (`{`), `TOKEN_FECHA_CHAVE` (`}`), `TOKEN_ABRE_PAR` (`(`), `TOKEN_FECHA_PAR` (`)`), `TOKEN_VIRGULA` (`,`), `TOKEN_PTOVIRG` (`;`)

---

## 6. Especificação das Regras a Implementar

Você deve modelar no Bison as regras sintáticas que atendam aos seguintes requisitos:

1. **Ponto de Entrada do Programa (`%start programa`):**
   - Um programa é composto por uma lista de uma ou mais declarações de classes.

2. **Declaração de Classes (`declaracao_classe`):**
   - Opcionalmente precedida por modificadores de acesso/escopo.
   - Palavra-chave `class`, nome da classe (`ID`) e corpo entre chaves `{ ... }`.

3. **Modificadores (`modificadores`, `modificador`):**
   - Suporte a combinações válidas de `public`, `private`, `protected`, `static`, `final`.

4. **Corpo da Classe e Membros (`corpo_classe`, `membro_classe`):**
   - O corpo da classe pode conter zero ou mais membros.
   - Cada membro pode ser uma declaração de método ou um atributo de classe (campo de variável com ou sem modificadores).

5. **Declaração de Métodos (`declaracao_metodo`):**
   - Modificadores opcionais (ex.: `public static`).
   - Tipo de retorno: um tipo de dado válido ou `void`.
   - Nome do método (`ID`) seguido por lista de parâmetros formais entre parênteses `( ... )`.
   - Corpo do método composto por um `bloco` entre chaves `{ ... }`.

6. **Lista de Parâmetros Formais (`lista_parametros`, `parametro`):**
   - Parâmetros separados por vírgula.
   - Suporte a parâmetros simples (`tipo ID`) e parâmetros do tipo array, especificamente permitindo a assinatura padrão do método `main`: `public static void main(String[] args)`.

*(Nota: a implementação das produções Bison fica a cargo do responsável pela issue; a referência técnica consolidada está disponível em `rep.md`)*

---

## 7. Casos de Teste Obrigatórios

Crie a pasta de testes: `tests/parser/issue_4/`

### 7.1 Testes Positivos (Devem ser aceitos com código 0)
1. `valido_classe_simples.java`:
   ```java
   class Exemplo {
   }
   ```
2. `valido_metodo_main.java`:
   ```java
   public class Principal {
       public static void main(String[] args) {
       }
   }
   ```
3. `valido_metodos_e_membros.java`:
   ```java
   public class Calculadora {
       private int resultado;

       public int somar(int a, int b) {
       }

       protected void resetar() {
       }
   }
   ```
4. `valido_multiplas_classes.java`:
   ```java
   class Auxiliar {
       static void ajuda() {
       }
   }

   public class Programa {
       public static void main(String[] args) {
       }
   }
   ```

### 7.2 Testes Negativos (Devem falhar com erro sintático)
1. `invalido_class_sem_nome.java`:
   ```java
   public class {
   }
   ```
2. `invalido_metodo_sem_retorno.java`:
   ```java
   public class Teste {
       somar(int a) {
       }
   }
   ```
3. `invalido_metodo_sem_parenteses.java`:
   ```java
   public class Teste {
       void executar {
       }
   }
   ```
4. `invalido_chaves_desbalanceadas.java`:
   ```java
   public class Teste {
       void executar() {
   }
   ```

---

## 8. Requisitos de Aceite (Definition of Done)

Para que esta issue seja aprovada e mergeada:

- [ ] **Fluxo Git & Pull Request:**
  - [ ] Trabalho realizado na branch dedicada `feat/issue-4-estrutura-programa` criada a partir da `main`.
  - [ ] Pull Request (PR) aberto no repositório apontando para a branch `main`.
  - [ ] Descrição do PR preenchida com as regras implementadas e evidência dos testes executados.
- [ ] **Documentação:**
  - [ ] Arquivo `docs/grammar/estrutura_programa.md` detalhando:
    - Regras BNF completas de programas, classes, modificadores e métodos.
    - Como o método `public static void main(String[] args)` é suportado gramaticalmente.
    - Como a regra `programa` orquestra os demais módulos no `%start`.
- [ ] **Testes:**
  - [ ] Criação dos arquivos `.java` da seção 7 dentro de `tests/parser/issue_4/`.
  - [ ] Script ou comando automatizado executando os testes:
    - Todos os arquivos válidos com classes e assinaturas compilam com código `0`.
    - Declarações malformadas emitem `Erro sintatico` e encerram com código != 0.
- [ ] **Compilação e Qualidade:**
  - [ ] Sem conflitos Shift/Reduce ou Reduce/Reduce no Bison.
  - [ ] Integração pronta para que, no merge final, os stubs de `bloco` e `tipo` sejam substituídos pelas regras reais das outras issues sem necessidade de refatorar a estrutura da classe.
