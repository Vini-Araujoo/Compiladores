# Limitações e Funcionalidades Fora do Escopo

Este documento lista as funcionalidades e construções da linguagem Java padrão que **não estão presentes** ou **não são suportadas** na gramática, na árvore sintática (AST) ou no compilador atual.

---

## 1. Orientação a Objetos Avançada

* **Herança e Polimorfismo:** Não há suporte a herança de classes (`extends`), interfaces (`implements`) nem modificador `abstract`.
* **Instanciação de Objetos de Usuário:** Não é suportado instanciar objetos de classes criadas pelo próprio usuário (ex.: `MinhaClasse obj = new MinhaClasse();`).
* **Construtores e Ponteiro `this`:** Não há suporte a declaração de construtores explícitos nem ao uso da palavra-chave `this`.
* **Sobrecarga de Métodos:** A tabela de símbolos e a semântica identificam métodos unicamente pelo nome simples (não há resolução de sobrecarga por assinatura de parâmetros diferentes).
* **Múltiplas Classes por Arquivo:** A gramática espera exatamente uma classe por arquivo fonte `.java`.

---

## 2. Tipos e Manipulação de Arrays

* **Declaração de Variáveis do Tipo Array:**
  * Não há suporte sintático na gramática (`parser.y`) para declarar variáveis locais ou de classe como vetores. Ambas as formas Java `int[] v;` e estilo C `int v[];` geram erro de sintaxe (*syntax error*).
  * O uso de colchetes na declaração é aceito **exclusivamente na lista de parâmetros de métodos** (ex.: `String[] args` ou `int[] param`), na expressão de alocação `new int[N]` e na expressão de indexação `v[i]`.
* **Instanciação de Arrays de outros tipos primitivos via `new`:**
  * O parser aceita instanciação com `new` apenas para `int`, `double` e `String` (ex.: `new int[10]`, `new double[5]`, `new String[3]`).
  * Instanciação dinâmica com `new` para tipos como `float`, `boolean`, `char`, `long`, `byte` e `short` (ex.: `new boolean[10]`) não está contemplada na regra `tipo_base` da gramática.
* **Arrays Multidimensionais:** Suporte restrito a arrays unidimensionais (vetores de uma dimensão). Declarações como `int[][] matriz` ou acessos `matriz[i][j]` não são suportados.
* **Métodos com retorno de Array:** A regra de declaração de método aceita apenas tipos primitivos, `String` ou `void`. Assinaturas com colchetes no retorno (ex.: `int[] obterValores()`) não são aceitas pela gramática.
* **Propriedade `.length` de Arrays:** Não há suporte sintático/semântico para leitura direta da propriedade `vetor.length`.

---

## 3. Operadores e Expressões

* **Operadores de Atribuição Composta:**
  * A gramática suporta apenas `=`, `+=` e `-=`.
  * Não há suporte léxico/sintático no parser para `*=`, `/=` e `%=`.
* **Operadores Bitwise e Deslocamento:**
  * Operadores a nível de bits (`&`, `|`, `^`, `~`, `<<`, `>>`, `>>>`) não estão implementados.
* **Operador Ternário:** A construção condicional ternária `condicao ? valor1 : valor2` não está presente na gramática.
* **Operador `instanceof`:** Não está presente na linguagem.
* **Casting Explícito de Tipos:** Conversões manuais de tipo na sintaxe `(int) expressao` ou `(double) expressao` não possuem regras no parser (o compilador realiza apenas conversões implícitas de alargamento numérico / widening).
* **Concatenação de Strings com outros Tipos (`+`):** Em Java, o operador `+` envolvendo `String` realiza concatenação e conversão dinâmica de primitivos (ex.: `"valor: " + x`). Como o compilador gera código C puro sem sobrecarga de operadores ou runtime complexo de strings, o operador `+` é emitido diretamente em C. Em C, somar `char*` com `int` resulta em aritmética de ponteiros (deslocamento de endereço de memória) em vez de concatenação textual. A impressão de múltiplos tipos na mesma linha deve ser realizada encadeando `System.out.print(...)` seguido de `System.out.println(...)`.

---

## 4. Estruturas de Controle de Fluxo

* **Laço For-Each:** Não há suporte à sintaxe iteradora `for (Tipo elemento : colecao)`.
* **Posicionamento do `default` no `switch`:** Na gramática, a cláusula `default:` precisa obrigatoriamente estar posicionada ao final dos blocos `case`.
* **Tratamento de Exceções:** Não há suporte a blocos `try`, `catch`, `finally`, nem comandos `throw` ou cláusulas `throws`.
* **Rótulos (Labels):** Não há suporte a `break` ou `continue` rotulados (ex.: `break label;`).

---

## 5. Bibliotecas e Pacotes (I/O)

* **Importação de Pacotes:** Não há suporte a declarações `import` ou `package`. Qualquer linha com `import` (inclusive o habitual `import java.util.Scanner;`) gera erro sintático logo no início da análise. No compilador, tipos como `Scanner` e comandos como `System.out` são tratados como primitivas nativas e não requerem importação.
* **Classes da API Padrão Java:**
  * Suporte restrito a `System.out.print`, `System.out.println` e instanciação padrão `new Scanner(System.in)`.
  * Não há suporte a outras classes da biblioteca padrão (ex.: `Math`, `ArrayList`, `List`, `HashMap`, `String.format`, leitura de arquivos via `FileReader`, etc.).
