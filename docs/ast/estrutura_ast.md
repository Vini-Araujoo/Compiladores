# Estrutura da Árvore Sintática Abstrata (AST)

Documentação técnica da infraestrutura básica da AST para o compilador Java-para-C (Issue 6.1).

---

## 1. Visão Geral

A AST (Árvore Sintática Abstrata) é a representação intermediária em memória do programa Java parseado. Diferente de uma árvore binária simples (que atende apenas expressões aritméticas), a AST deste compilador utiliza uma **árvore N-ária genérica**, na qual cada nó pode conter múltiplos nós filhos por meio de um vetor dinamicamente redimensionável.

Essa estrutura permite representar com fidelidade construções de alto nível da linguagem Java, tais como classes com múltiplos atributos/métodos, blocos de comandos com dezenas de instruções e métodos com listas de parâmetros.

---

## 2. Tipos e Enums (`src/tipos.h`)

### `TipoDado`
Representa os tipos de dados suportados pelo compilador para fins de tipagem e futura análise semântica:
- `TIPO_VOID`: Ausência de retorno em métodos.
- Tipos Primitivos Inteiros: `TIPO_BYTE`, `TIPO_SHORT`, `TIPO_INT`, `TIPO_LONG`.
- Tipos Primitivos Ponto Flutuante: `TIPO_FLOAT`, `TIPO_DOUBLE`.
- Outros Primitivos: `TIPO_CHAR`, `TIPO_BOOLEAN`.
- Tipos Especiais: `TIPO_STRING`, `TIPO_ARRAY`, `TIPO_SCANNER`.

### `TipoNoAST`
Classifica o papel sintático de cada nó na árvore:
* **Estrutura de Classes**: `NO_PROGRAMA`, `NO_CLASSE`, `NO_METODO`, `NO_PARAMETRO`, `NO_VAR_DECL`, `NO_BLOCO`.
* **Controle de Fluxo**: `NO_IF`, `NO_WHILE`, `NO_FOR`, `NO_DO_WHILE`, `NO_SWITCH`, `NO_CASE`, `NO_DEFAULT`, `NO_RETURN`, `NO_BREAK`.
* **Comandos e I/O**: `NO_ATRIBUICAO`, `NO_PRINT`, `NO_PRINTLN`, `NO_SCANNER`.
* **Expressões e Operações**: `NO_OP_BINARIA`, `NO_OP_UNARIA`, `NO_CHAMADA_METODO`, `NO_NOVA_INSTANCIA`, `NO_ACESSO_ARRAY`.
* **Literais e Folhas**: `NO_IDENTIFICADOR`, `NO_LITERAL_INT`, `NO_LITERAL_FLOAT`, `NO_LITERAL_DOUBLE`, `NO_LITERAL_CHAR`, `NO_LITERAL_STRING`, `NO_LITERAL_BOOL`.

---

## 3. Estrutura do Nó (`src/ast.h`)

Cada nó é uma instância de `struct NoAST`:

```c
typedef struct NoAST {
    TipoNoAST tipo_no;         /* Papel sintático do nó */
    TipoDado tipo_dado;        /* Tipo associado (ex: TIPO_INT, TIPO_VOID) */
    ValorLiteral valor;        /* União com valor de literais, strings ou operadores */
    int linha;                 /* Linha do arquivo fonte Java correspondente */

    struct NoAST **filhos;     /* Vetor dinâmico de ponteiros para nós filhos */
    int num_filhos;            /* Quantidade de filhos atualmente anexados */
    int capacidade;            /* Capacidade total alocada no vetor de filhos */
} NoAST;
```

### União de Valores (`ValorLiteral`)
Economiza memória compartilhando o espaço para dados específicos de cada tipo de nó folha/expressão:
```c
typedef union {
    long valor_int;
    double valor_double;
    char *valor_str;
    char operador[4];
} ValorLiteral;
```

---

## 4. Gerenciamento Dinâmico de Filhos (`src/ast.c`)

### `NoAST *criarNo(TipoNoAST tipo, int linha)`
* Aloca a estrutura do nó com `malloc`.
* Inicializa os campos (`tipo_no`, `linha`, `num_filhos = 0`).
* Inicializa o vetor de filhos com capacidade inicial `CAPACIDADE_INICIAL_FILHOS = 4`.
* Retorna o ponteiro para o nó recém-criado.

### `void adicionarFilho(NoAST *pai, NoAST *filho)`
* **Tratamento seguro para nulos**: Caso `pai` ou `filho` seja `NULL`, a função retorna imediatamente sem falhar ou causar `Segmentation Fault`.
* **Redimensionamento dinâmico**: Se `pai->num_filhos >= pai->capacidade`, a capacidade é dobrada através de `realloc` exponencial ($4 \to 8 \to 16 \dots$).
* O ponteiro do filho é anexado na posição `pai->filhos[pai->num_filhos++]`.

### `void liberarAST(NoAST *raiz)`
* Percorre recursivamente todos os nós filhos liberando memória em pós-ordem.
* Libera o vetor de ponteiros `raiz->filhos`.
* Libera a estrutura `raiz`.

---

## 5. Testes Unitários

A suíte da AST possui testes dedicados em `tests/ast/`:
1. **`teste_criacao_nos.c`**: Valida criação de nós raiz, classes, métodos e inserção de múltiplos filhos com expansão de capacidade além do limite inicial.
2. **`teste_adicionar_filho_nulo.c`**: Valida tratamento defensivo contra filhos ou pais nulos.

Para compilar e executar os testes:
```bash
make test-ast
```
