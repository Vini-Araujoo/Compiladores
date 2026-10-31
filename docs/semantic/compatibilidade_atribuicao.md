# Compatibilidade de Tipos em Atribuições e Coerção Implícita (Issue 8.5)

Documentação técnica da implementação da verificação de compatibilidade de tipos, promoções por alargamento (*widening primitive conversions*) e bloqueio de atribuições inválidas (*narrowing* ou tipos incompatíveis) no compilador Java.

---

## 1. Identificação da Issue

- **Responsável:** Vini-Araujoo
- **Branch:** `feat/issue-8.5-semantica-atribuicao`
- **Módulo:** Análise Semântica de Tipos
- **Status:** Concluída

---

## 2. Visão Geral e Objetivos

Na linguagem Java (conforme a *Java Language Specification* - JLS §5.1.2 e §5.2), o tipo de uma expressão atribuída a uma variável deve ser semanticamente compatível com o tipo declarado do destino.

Os objetivos desta implementação são:
1. Implementar e disponibilizar a função pública:
   ```c
   int tiposCompativeis(TipoDado destino, TipoDado origem);
   ```
2. Implementar a função de conversão de tipos para texto descritivo:
   ```c
   const char *nomeTipoDado(TipoDado tipo);
   ```
3. Aceitar conversões implícitas de alargamento (*widening primitive conversions*) permitidas pela especificação da linguagem Java.
4. Bloquear conversões de estreitamento (*narrowing primitive conversions*) sem *cast* explícito (ex.: atribuir `double` para `int`).
5. Bloquear atribuições entre tipos disjuntos/incompatíveis (ex.: atribuir `boolean` ou `String` para `int`, ou `String` para `boolean`).
6. Validar compatibilidade de tipos tanto na inicialização de variáveis (`NO_DECL_VARIAVEL` com expressão) quanto em comandos de atribuição direta e composta (`NO_ATRIBUICAO`).
7. Emitir mensagens de erro semântico claras e padronizadas contendo o número da linha, o identificador do destino, o tipo esperado e o tipo fornecido.

---

## 3. Matriz de Compatibilidade em Atribuições

A tabela a seguir apresenta a matriz de compatibilidade semântica direta para atribuições (`destino = origem;`), mapeando todos os tipos primitivos e tipos de dados suportados:

| Destino \ Origem | `byte` | `short` | `char` | `int` | `long` | `float` | `double` | `boolean` | `String` |
|:---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **`byte`** | Sim | Não | Não | Não | Não | Não | Não | Não | Não |
| **`short`** | **Sim** | Sim | Não | Não | Não | Não | Não | Não | Não |
| **`char`** | Não | Não | Sim | Não | Não | Não | Não | Não | Não |
| **`int`** | **Sim** | **Sim** | **Sim** | Sim | Não | Não | Não | Não | Não |
| **`long`** | **Sim** | **Sim** | **Sim** | **Sim** | Sim | Não | Não | Não | Não |
| **`float`** | **Sim** | **Sim** | **Sim** | **Sim** | **Sim** | Sim | Não | Não | Não |
| **`double`** | **Sim** | **Sim** | **Sim** | **Sim** | **Sim** | **Sim** | Sim | Não | Não |
| **`boolean`**| Não | Não | Não | Não | Não | Não | Não | Sim | Não |
| **`String`** | Não | Não | Não | Não | Não | Não | Não | Não | Sim |

### Legenda e Decisões Técnicas:
- **Sim (Identidade):** Tipos idênticos (`destino == origem`) são sempre compatíveis.
- **Sim (Widening / Alargamento):** Conversão implícita sem perda de magnitude autorizada pela JLS §5.1.2:
  - `byte` $\rightarrow$ `short`, `int`, `long`, `float`, `double`
  - `short` $\rightarrow$ `int`, `long`, `float`, `double`
  - `char` $\rightarrow$ `int`, `long`, `float`, `double`
  - `int` $\rightarrow$ `long`, `float`, `double`
  - `long` $\rightarrow$ `float`, `double`
  - `float` $\rightarrow$ `double`
- **Não (Narrowing sem cast):** Conversão com potencial perda de dados requer *cast* explícito e é terminantemente proibida em atribuições implícitas simples:
  - `double` para `int`/`float`/`long`/`short`/`byte`/`char`
  - `float` para tipos inteiros
  - `long` para `int`/`short`/`byte`/`char`
  - `int` para `short`/`byte`/`char`
  - `short` para `byte`/`char`
  - `char` para `short` ou `byte` (em Java, `char` é não sinalizado de 16 bits, enquanto `short` é sinalizado de 16 bits).
- **Não (Incompatibilidade estrita):** Tipos incompatíveis por definição semântica (ex.: `boolean` não pode ser convertido para numérico e vice-versa; `String` só pode receber literal ou expressão `String` em operador `=`).

---

## 4. Arquitetura e Implementação

### 4.1 Interface Pública (`src/semantica.h`)

Foram exportadas duas novas funções para uso de outros módulos do compilador e da suíte de testes unitários:

```c
/* Compatibilidade de tipos e coerção por alargamento (widening) do Java (Issue 8.5) */
int tiposCompativeis(TipoDado destino, TipoDado origem);
const char *nomeTipoDado(TipoDado tipo);
```

### 4.2 Função de Compatibilidade (`src/semantica.c`)

A função `tiposCompativeis` avalia a compatibilidade seguindo rigorosamente a matriz formal:

```c
int tiposCompativeis(TipoDado destino, TipoDado origem) {
    if (destino == origem) return 1;
    switch (origem) {
        case TIPO_BYTE:
            return (destino == TIPO_SHORT || destino == TIPO_INT || destino == TIPO_LONG ||
                    destino == TIPO_FLOAT || destino == TIPO_DOUBLE);
        case TIPO_SHORT:
            return (destino == TIPO_INT || destino == TIPO_LONG ||
                    destino == TIPO_FLOAT || destino == TIPO_DOUBLE);
        case TIPO_CHAR:
            return (destino == TIPO_INT || destino == TIPO_LONG ||
                    destino == TIPO_FLOAT || destino == TIPO_DOUBLE);
        case TIPO_INT:
            return (destino == TIPO_LONG || destino == TIPO_FLOAT || destino == TIPO_DOUBLE);
        case TIPO_LONG:
            return (destino == TIPO_FLOAT || destino == TIPO_DOUBLE);
        case TIPO_FLOAT:
            return (destino == TIPO_DOUBLE);
        default:
            return 0;
    }
}
```

### 4.3 Inferência de Tipos e Tratamento de Interdependências

Para evitar acoplamento com tarefas paralelas (como a Issue 9.3), a função `inferirTipoExpressao` foi desenhada para resolver localmente:
- **Literais primitivos:** `NO_LITERAL_INT`, `NO_LITERAL_FLOAT`, `NO_LITERAL_DOUBLE`, `NO_LITERAL_STRING`, `NO_LITERAL_CHAR`, `NO_LITERAL_BOOL`.
- **Identificadores:** Consulta de tipo na Tabela de Símbolos ou no próprio ponteiro `no->simbolo`.
- **Operações unárias:** Negação lógica (`!`) retorna `TIPO_BOOLEAN`, operações numéricas retornam o tipo do operando.
- **Operações binárias:** Operadores relacionais e lógicos retornam `TIPO_BOOLEAN`; concatenação de `String` com `+` retorna `TIPO_STRING`; expressões aritméticas seguem promoção binária básica (`double` > `float` > `long` > `int`).
- **Nós com anotação prévia:** Caso o nó já possua `tipo_dado` definido (stubs de teste ou gerado em nós sintetizados), o tipo prévio é respeitado.

### 4.4 Verificação Semântica na AST

A validação semântica atua em dois momentos principais durante a travessia:

1. **Inicialização na Declaração de Variáveis (`verificarDeclaracao`):**
   ```c
   if (no->num_filhos > 0 && no->filhos[0]) {
       TipoDado tipo_expr = inferirTipoExpressao(no->filhos[0]);
       if (tipo_expr != TIPO_VOID && !tiposCompativeis(no->tipo_dado, tipo_expr)) {
           fprintf(stderr, "Erro semantico na linha %d: tipos incompativeis na inicializacao de '%s'. Esperado '%s', recebido '%s'.\n",
                   linha, nome, nomeTipoDado(no->tipo_dado), nomeTipoDado(tipo_expr));
           erros_escopo++;
       }
   }
   ```

2. **Atribuições (`NO_ATRIBUICAO`):**
   ```c
   if (no->num_filhos > 1 && no->filhos[1]) {
       TipoDado tipo_origem = inferirTipoExpressao(no->filhos[1]);
       TipoDado tipo_destino = s->tipo;
       const char *op = no->valor.operador;
       int compativel = 1;

       if (op && strcmp(op, "+=") == 0 && tipo_destino == TIPO_STRING) {
           compativel = 1; /* Concatenação acumulada em String */
       } else if (tipo_origem != TIPO_VOID) {
           compativel = tiposCompativeis(tipo_destino, tipo_origem);
       }

       if (!compativel) {
           fprintf(stderr, "Erro semantico na linha %d: tipos incompativeis na atribuicao para '%s'. Esperado '%s', recebido '%s'.\n",
                   linha, s->nome, nomeTipoDado(tipo_destino), nomeTipoDado(tipo_origem));
           erros_escopo++;
       }
   }
   ```

---

## 5. Mensagens de Erro Semântico

As mensagens seguem a convenção com indicação exata de linha e descrição dos tipos:

### 5.1 Atribuição Incompatível
```text
Erro semantico na linha <linha>: tipos incompativeis na atribuicao para '<nome>'. Esperado '<tipo_esperado>', recebido '<tipo_fornecido>'.
```

### 5.2 Inicialização Incompatível
```text
Erro semantico na linha <linha>: tipos incompativeis na inicializacao de '<nome>'. Esperado '<tipo_esperado>', recebido '<tipo_fornecido>'.
```

---

## 6. Cobertura de Testes Automatizados

### 6.1 Testes Unitários em C (`tests/semantic/teste_escopos_unitario.c`)
- `teste_tipos_compativeis_matriz`: Validação exaustiva de todas as combinações válidas de *widening*, bloqueio de *narrowing* e tipos disjuntos.
- `teste_atribuicao_e_inicializacao_tipos`: Construção de ASTs com casos de coerção positiva (`int` $\rightarrow$ `double`), *narrowing* negativo (`double` $\rightarrow$ `int`) e tipos incompatíveis (`String` $\rightarrow$ `boolean`).

### 6.2 Testes Automatizados em Arquivos Java (`tests/semantic/`)
- `valido_coercao_int_para_double.java`: Atribuição válida de valor inteiro para variável do tipo `double` (retorno com código de saída 0).
- `invalido_atribuicao_double_para_int.java`: Rejeição semântica com erro na linha 5 ao atribuir `double` para `int`.
- `invalido_atribuicao_string_para_boolean.java`: Rejeição semântica com erro na linha 4 ao atribuir `String` para `boolean`.

### 6.3 Execução das Suítes
Todas as suítes são executadas automaticamente via:
```bash
make test-semantic
```
Garantindo zero *warnings* do GCC (`-Wall -Wextra`) e zero conflitos Shift/Reduce ou Reduce/Reduce no Bison.
