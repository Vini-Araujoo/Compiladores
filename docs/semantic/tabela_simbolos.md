# Tabela de Símbolos e Pilha de Escopos (Issue 8.1)

Documentação técnica da infraestrutura da Tabela de Símbolos baseada em pilha hierárquica de escopos para o compilador Java-para-C.

---

## 1. Visão Geral

A **Tabela de Símbolos** é o componente central da fase de análise semântica. Ela é responsável por registrar identificadores declarados (variáveis, parâmetros, constantes) juntamente com seus metadados (tipo de dado, modificadores e linha de declaração) e resolver consultas de identificadores respeitando as regras de visibilidade e escopo léxico da linguagem Java.

Para suportar blocos aninhados, métodos e laços de repetição, a tabela foi implementada como uma **pilha encadeada de escopos** (árvore estática de escopos com ponteiros para o escopo pai). Cada escopo mantém sua própria tabela hash para inserção e consulta local de tempo constante $O(1)$ médio, enquanto consultas globais utilizam **busca ascendente** a partir do escopo corrente em direção à raiz.

---

## 2. Estruturas de Dados (`src/tabela.h`)

### 2.1 `Modificadores`
Armazena flags booleanas correspondentes aos modificadores Java aceitos pela gramática:
```c
typedef struct {
    int eh_public;
    int eh_private;
    int eh_protected;
    int eh_static;
    int eh_final;
} Modificadores;
```

### 2.2 `Simbolo`
Representa um identificador registrado em um escopo específico:
```c
typedef struct Simbolo {
    char *nome;              /* Nome textual do identificador alocado dinamicamente */
    TipoDado tipo;           /* Tipo associado (ex: TIPO_INT, TIPO_DOUBLE, etc.) */
    Modificadores mods;      /* Modificadores de visibilidade e mutabilidade */
    int linha;               /* Linha de declaracao no codigo-fonte original */
    struct Simbolo *proximo; /* Ponteiro para colisao na tabela hash */
} Simbolo;
```

### 2.3 `Escopo`
Representa um nível de escopo léxico ativo:
```c
#define TAM_TABELA_HASH 31

typedef struct Escopo {
    Simbolo *tabela[TAM_TABELA_HASH]; /* Buckets da tabela hash local */
    struct Escopo *pai;                /* Ponteiro para o escopo pai (enclosing scope) */
    int nivel;                         /* Nivel de aninhamento (0 = global) */
} Escopo;
```

---

## 3. Arquitetura da Pilha e Diagrama Conceitual

A pilha de escopos cresce conforme o compilador entra em métodos e blocos de comandos `{ ... }`, e encolhe ao sair deles:

```text
    +-----------------------------------------------+
    | Escopo Local (Nível 2 - Bloco if / for)       |
    | - tabela hash: { "i": TIPO_INT (linha 15) }   |
    | - pai -------------------------------------+  |
    +--------------------------------------------|--+
                                                 v
    +-----------------------------------------------+
    | Escopo de Método (Nível 1 - void calcular())  |
    | - tabela hash: { "x": TIPO_DOUBLE, "a": ... } |
    | - pai -------------------------------------+  |
    +--------------------------------------------|--+
                                                 v
    +-----------------------------------------------+
    | Escopo de Classe / Global (Nível 0)           |
    | - tabela hash: { "x": TIPO_INT, "PI": ... }   |
    | - pai = NULL (Raiz da hierarquia)             |
    +-----------------------------------------------+
```

---

## 4. Algoritmos e Operações

### 4.1 Função de Espalhamento (Hash)
Utiliza a função canônica **djb2**, garantindo dispersão uniforme dos identificadores entre os 31 buckets de cada escopo:
$$hash = ((hash \ll 5) + hash) + c = hash \times 33 + c$$

### 4.2 Abertura de Escopo (`empilharEscopo`)
1. Aloca dinamicamente uma nova `struct Escopo`.
2. Zera todos os buckets da tabela hash local (`tabela[i] = NULL`).
3. Define o ponteiro `pai` apontando para o `escopo_atual`.
4. Define `nivel = escopo_atual ? escopo_atual->nivel + 1 : 0`.
5. Atualiza o ponteiro global `escopo_atual` para o novo nó alocado.

### 4.3 Fechamento de Escopo (`desempilharEscopo`)
1. Salva o ponteiro do escopo que será fechado (`escopo_remover = escopo_atual`).
2. Atualiza o topo da pilha para o escopo pai: `escopo_atual = escopo_atual->pai`.
3. Percorre todos os buckets da hash de `escopo_remover`, liberando a string `nome` e a estrutura de cada `Simbolo`.
4. Desaloca a estrutura `Escopo`.
5. Garante que variáveis locais declaradas naquele bloco tornem-se inalcançáveis e não causem vazamento de memória (*memory leaks*).

### 4.4 Inserção com Validação no Escopo Corrente (`inserirSimbolo`)
```c
int inserirSimbolo(char *nome, TipoDado tipo, Modificadores mods, int linha);
```
1. Verifica se já existe um símbolo com o mesmo identificador no escopo corrente através de `buscarSimboloEscopoAtual(nome)`.
2. **Se existir no mesmo escopo:** Rejeita a operação e retorna `0`. Em Java, a redeclaração de uma variável dentro do mesmo bloco é proibida.
3. **Se não existir no escopo corrente:** Aloca e insere o `Simbolo` no bucket correspondente do `escopo_atual`, retornando `1`.
4. **Sombreamento (Shadowing):** Se a variável já existir em um escopo pai (mais externo), a inserção no escopo atual é permitida. A busca priorizará o escopo mais interno, mascarando o símbolo do pai enquanto o escopo interno estiver ativo.

### 4.5 Algoritmo de Busca Ascendente (`buscarSimbolo`)
```c
Simbolo *buscarSimbolo(char *nome);
```
1. Inicializa o ponteiro de busca: `e = escopo_atual`.
2. Enquanto `e != NULL`:
   - Calcula o índice hash de `nome`.
   - Percorre a lista encadeada do bucket `e->tabela[indice]`.
   - Se encontrar um símbolo com `strcmp(simb->nome, nome) == 0`, retorna o ponteiro para o `Simbolo`.
   - Caso contrário, sobe para o escopo pai: `e = e->pai`.
3. Se atingir o fim da cadeia (`e == NULL`) sem encontrar, retorna `NULL`.

---

## 5. Análise de Complexidade

| Operação | Caso Médio | Pior Caso | Espaço |
| :--- | :---: | :---: | :---: |
| `empilharEscopo()` | $O(1)$ | $O(1)$ | $O(1)$ |
| `desempilharEscopo()` | $O(K)$ | $O(K)$ | $O(1)$ |
| `inserirSimbolo()` | $O(1)$ | $O(N_{escopo})$ | $O(1)$ |
| `buscarSimboloEscopoAtual()` | $O(1)$ | $O(N_{escopo})$ | $O(1)$ |
| `buscarSimbolo()` | $O(P)$ | $O(P \times N_{escopo})$ | $O(1)$ |

*Onde $K$ é o número de símbolos do escopo desempilhado, $N_{escopo}$ é a quantidade de variáveis no escopo, e $P$ é a profundidade atual da pilha de escopos (geralmente $P \le 5$).*

---

## 6. Cobertura de Testes Unitários

A suíte de testes unitários foi implementada em C na pasta `tests/semantic/`:

1. **`teste_pilha_escopos.c` (Positivo):**
   - Criação e validação do escopo global (Nível 0).
   - Empilhamento de escopo de método (Nível 1) e bloco aninhado (Nível 2).
   - Busca ascendente localizando identificadores globais a partir de blocos aninhados.
   - Sombreamento de variável (`g_int`) em 3 níveis distintos, verificando que cada nível recupera seu tipo correspondente.
   - Desempilhamento sucessivo restaurando o valor do escopo pai anterior.
   - Liberação completa de memória.

2. **`teste_busca_apos_fechamento_escopo.c` (Negativo):**
   - Valida que identificadores alocados em escopos locais tornam-se estritamente inacessíveis (`NULL`) após o fechamento do bloco.
   - Valida que tentativas de redeclaração de variáveis com o mesmo identificador no mesmo escopo corrente são rejeitadas com retorno `0` e mensagem semântica informativa indicando a linha.

### Execução dos Testes:
```bash
make test-tabela
# ou
make test-semantic
```
Ambos executam a compilação estrita (`-Wall -Wextra`) e executam os testes com código de saída 0.
