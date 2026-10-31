# Resolução de Nomes e Escopos de Variáveis (Issue 8.3)

Documentação técnica da passagem semântica de cadastro de símbolos, resolução de escopo e verificação de identificadores para o compilador Java-para-C.

---

## 1. Identificação da Issue

- **Responsável:** Vini-Araujoo
- **Branch:** `feat/issue-8.3-resolucao-variaveis`
- **Módulo:** Análise Semântica de Escopo
- **Status:** Concluída

---

## 2. Visão Geral

A passagem de **Resolução de Nomes e Escopos** (`verificarEscopos`) é responsável por percorrer a Árvore Sintática Abstrata (AST) construída pelo parser, gerenciar a pilha hierárquica de escopos e validar as regras léxicas da linguagem:
1. **Cadastro de Variáveis (`NO_DECL_VARIAVEL`):** insere variáveis no escopo corrente e emite erro semântico caso já exista símbolo homônimo no mesmo bloco.
2. **Resolução de Identificadores (`NO_IDENTIFICADOR` / `NO_ID`):** busca o símbolo na cadeia ascendente de escopos, emitindo erro caso a variável não tenha sido declarada.
3. **Associação de Ponteiro para o Codegen (`no->simbolo`):** liga diretamente cada nó identificador e declaração à respectiva entrada `Simbolo` na tabela de símbolos, viabilizando consultas diretas de tipo, modificadores e deslocamento na fase de geração de código.

---

## 3. Regras de Visibilidade e Escopo

### 3.1 Hierarquia de Escopos

```text
  [Nível 0: Escopo de Classe / Global]
      ├── Atributos de Classe (campos)
      │
      └── [Nível 1: Escopo de Método]
              ├── Parâmetros Formais
              ├── Variáveis Locais do Método
              │
              └── [Nível 2: Bloco Aninhado ou Laço For]
                      ├── Variáveis Locais do Bloco
                      └── [Nível 3: Sub-blocos...]
```

### 3.2 Escopo de Classe / Membros Globais (`NO_CLASSE`)
- Os campos e atributos da classe residem no escopo raiz (nível 0).
- **Processamento em Duas Passagens:**
  - *Passagem 1:* Registra antecipadamente todas as declarações de atributos da classe (`NO_DECL_VARIAVEL`). Isso assegura que métodos possam referenciar campos declarados textualmente abaixo deles no arquivo-fonte, em conformidade com as regras de classes em Java.
  - *Passagem 2:* Analisa as expressões de inicialização dos atributos e os métodos da classe (`NO_METODO`).

### 3.3 Escopo de Métodos (`NO_METODO`)
- A entrada em um método empilha um novo escopo (`empilharEscopo()`, nível 1).
- Os parâmetros formais (`NO_PARAMETROS`) são cadastrados no escopo do método.
- O bloco principal do método (`filhos[1]`) compartilha esse mesmo escopo. Portanto, declarar uma variável local com o mesmo nome de um parâmetro no nível raiz do método gera erro semântico de redeclaração no mesmo escopo.
- Ao término do método, o escopo é desempilhado (`desempilharEscopo()`).

### 3.4 Escopo de Blocos Aninhados (`NO_BLOCO`)
- Cada bloco interno `{ ... }` (ex.: comandos compostos em `if`, `while` ou blocos explícitos) empilha um novo escopo filho.
- **Sombreamento (Shadowing) Permitido:** Uma variável declarada em um bloco aninhado pode possuir o mesmo identificador de uma variável em escopos ancestrais (método ou classe). A busca ascendente prioriza a declaração mais interna.
- **Isolamento pós-fechamento:** Ao desempilhar o bloco, as variáveis declaradas nele deixam de ser visíveis para comandos subsequentes no escopo externo ou em blocos irmãos.

### 3.5 Escopo de Repetições (`NO_FOR`)
- O laço `for` empilha um escopo específico que engloba a inicialização (`filhos[0]`), a condição (`filhos[1]`), o incremento (`filhos[2]`) e o corpo (`filhos[3]`).
- Variáveis declaradas na cláusula de inicialização (ex.: `for (int i = 0; ...)`) possuem escopo restrito ao laço e tornam-se inacessíveis fora dele.

### 3.6 Proibição de Redeclaração no Mesmo Escopo
- É expressamente proibida a redeclaração de variáveis com o mesmo identificador dentro do mesmo nível de escopo corrente (`buscarSimboloEscopoAtual(nome) != NULL`).
- A infraestrutura rejeita a declaração e contabiliza erro semântico.

---

## 4. Estrutura de Dados e Preservação de Símbolos

### 4.1 Extensão do Nó da AST (`src/ast.h`)

Para viabilizar o backend e a geração de código (codegen), o nó `NoAST` recebeu o campo `simbolo`:

```c
struct Simbolo;

typedef struct NoAST
{
    TipoNoAST tipo_no;
    TipoDado tipo_dado;
    ValorLiteral valor;
    int linha;
    int posfixo;
    struct Simbolo *simbolo; /* Ponteiro direto para a entrada na Tabela de Símbolos */

    struct NoAST **filhos;
    int num_filhos;
    int capacidade;
} NoAST;
```

Em `src/ast.c`, a função `criarNo` inicializa `no->simbolo = NULL;`.

### 4.2 Preservação de Ponteiros para Codegen (`src/tabela.c`)

**Desafio Técnico:** Se `desempilharEscopo()` desalocasse imediatamente as estruturas `Simbolo` ao fechar um bloco ou método, qualquer ponteiro `no->simbolo` na AST apontaria para memória liberada (*dangling pointer*), causando falhas de segmentação no codegen.

**Solução Arquitetural:**
- A estrutura `Escopo` recebeu o campo `struct Escopo *proximo_alocado;`.
- Todas as instâncias de `Escopo` alocadas são encadeadas na lista global `todos_escopos`.
- `desempilharEscopo()` apenas retrocede o ponteiro ativo na hierarquia (`escopo_atual = escopo_atual->pai;`), mantendo o isolamento rigoroso de visibilidade sem desalocar a memória.
- Todos os ponteiros `no->simbolo` permanecem íntegros até a chamada final de `liberarTabelaSimbolos()` ao término da execução do compilador.

---

## 5. Tratamento de Erros Semânticos

As mensagens de erro seguem o padrão uniforme do compilador, emitidas em `stderr` com indicação precisa da linha de ocorrência:

### 5.1 Variável Não Declarada
Emitido ao tentar utilizar um identificador em expressões, atribuições, acessos ou chamadas sem que ele exista no escopo atual ou em qualquer escopo ancestral:
```text
Erro semantico na linha <linha>: variavel '<nome>' nao declarada.
```

### 5.2 Redeclaração no Mesmo Escopo
Emitido ao tentar declarar uma variável ou parâmetro com identificador já existente no mesmo escopo corrente:
```text
Erro semantico na linha <linha>: variavel '<nome>' ja declarada neste escopo.
```

---

## 6. Integração com o Parser

No arquivo [parser.y](file:///c:/Users/Vinícius/Documents/Compiladores/parser/parser.y), a função `main` suporta a flag de execução semântica:

```bash
# Execução sintática pura (padrão):
build/parser < arquivo.java

# Execução com validação semântica de escopo (retorna código de saída 1 em caso de erro semântico):
build/parser --semantico < arquivo.java
```

Essa separação garante total retrocompatibilidade com os testes sintáticos pré-existentes (`make test-ast`), enquanto permite validação semântica estrita nos testes de escopo (`make test-semantic`).

---

## 7. Testes e Validação

### 7.1 Teste Unitário em C (`tests/semantic/teste_escopos_unitario.c`)
Testa diretamente a função `verificarEscopos` construindo pequenas árvores sintáticas via stubs controlados com `criarNo`:
1. `teste_declaracao_e_shadowing_valido`: declaração em escopo global, método e bloco aninhado; validação da associação de ponteiros e distinção de instâncias no sombreamento.
2. `teste_variavel_nao_declarada`: uso de identificador não declarado; validação de rejeição (status 0).
3. `teste_redeclaracao_mesmo_escopo`: declarações duplicadas no mesmo bloco; validação de rejeição (status 0).
4. `teste_acesso_fora_do_escopo`: tentativa de acessar variável de bloco aninhado fechado; validação de isolamento e rejeição.
5. `teste_escopo_for`: validação de variável de inicialização do loop acessível na condição, incremento e bloco.

### 7.2 Casos de Teste em Java (`tests/semantic/`)
- [valido_escopos_aninhados_permitidos.java](file:///c:/Users/Vinícius/Documents/Compiladores/tests/semantic/valido_escopos_aninhados_permitidos.java): atributos de classe, parâmetros de método, variáveis locais e múltiplos blocos internos com sombreamento e escopos irmãos válidos (código de saída 0).
- [invalido_variavel_nao_declarada.java](file:///c:/Users/Vinícius/Documents/Compiladores/tests/semantic/invalido_variavel_nao_declarada.java): referência à variável `b` sem declaração; validação de rejeição na linha 5.
- [invalido_redeclaracao_mesmo_escopo.java](file:///c:/Users/Vinícius/Documents/Compiladores/tests/semantic/invalido_redeclaracao_mesmo_escopo.java): declaração duplicada da variável `a` no mesmo método; validação de rejeição na linha 5.

### 7.3 Comandos de Execução

```bash
# Executar suíte completa de análise semântica:
make test-semantic

# Executar suíte da AST:
make test-ast

# Executar suíte do parser sintático:
make test
```
