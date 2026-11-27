# Guia de Testes e Relatório de Validação da Tradução Java para C

Este documento descreve a suíte de testes de tradução da linguagem Java para C (transpilação fonte-para-fonte) localizada em `tests/compilador/`, documentando como executar os testes de geração de código C e o status detalhado de cada cenário de teste.

> [!NOTE]
> **Escopo do Projeto:** O objetivo deste projeto acadêmico de Compiladores é estritamente a **tradução de código-fonte Java para código-fonte C** (`.java` -> `.c`), atuando como um tradutor fonte-para-fonte (*source-to-source compiler* ou *transpilador*). O compilador desenvolvido gera exclusivamente código C na saída padrão. A compilação do C gerado com o GCC serve unicamente como verificação externa para atestar que o código C emitido é sintaticamente válido e semanticamente equivalente.

---

## 1. Como Executar os Testes de Tradução

O projeto conta com alvos automatizados no `Makefile` e scripts de execução modular.

### A. Executar a Bateria de Testes de Tradução (Java -> C)
Para rodar a verificação de tradução de todos os cenários da pasta `tests/compilador/`:
```bash
make test-compilador
# ou diretamente via script:
bash tests/compilador/run-tests.sh
```

### B. Executar Todas as Suítes do Projeto
Para validar todas as etapas do projeto (Léxico/Sintático, AST, Tabela de Símbolos, Semântico, Codegen e Tradutor CLI):
```bash
make test-all
```

### C. Executar as Suítes Modulares Separadamente por Etapa
```bash
make test           # 64 testes do Parser (Bison/Flex)
make test-ast       # 39 testes unitários da AST
make test-tabela    # 4 testes da Tabela de Símbolos e Escopos
make test-semantic  # 39 testes de Validação Semântica
make test-codegen   # 129 testes de Geração de Código em C
```

### D. Executar a Tradução de um Arquivo Java Individualmente
Para traduzir um arquivo Java específico e gerar o arquivo C correspondente:
```bash
./build/compilador tests/compilador/01_aritmetica_e_tipos.java > programa.c
```
*(Opcional: para inspecionar o código C gerado, basta abrir `programa.c` ou validá-lo com `gcc -Wall -Wextra -o main programa.c && ./main`).*

---

## 2. Relatório de Testes: O que Passa e o que Não Passa

A suíte em `tests/compilador/` possui **14 cenários** cobrindo a linguagem aceita, casos limites e diagnósticos de projeto.

### Resumo dos Resultados

| Nº | Teste | Arquivo Java | Código C Gerado | Status | Comportamento / Saída |
| :-: | :--- | :--- | :---: | :---: | :--- |
| **01** | Aritmética e Tipos | `01_aritmetica_e_tipos.java` | `01_aritmetica_e_tipos.c` | ✅ **PASSOU** | Precedência correta, tipos primitivos, `char 'X'`, `String` e `boolean`. Saída: `47`, `12.56`, `true`, `X`, `Compiladores 2026`. |
| **02** | Constantes `final` | `02_constantes_final.java` | `02_constantes_final.c` | ✅ **PASSOU** | Mapeamento de `final` para `const` em C e expressões combinadas. Saída: `150`, `0.05`. |
| **03** | Controle de Fluxo | `03_controle_fluxo_if_else.java` | `03_controle_fluxo_if_else.c` | ✅ **PASSOU** | `if`, `else if`, `else`, lógicos (`&&`, `||`, `!`) e operadores de igualdade (`==`, `!=`). Saída: `1`, `2`, `20`. |
| **04** | Laços de Repetição | `04_lacos_repeticao.java` | `04_lacos_repeticao.c` | ✅ **PASSOU** | Laços `while`, `do-while` e `for` com operadores `++` e `--`. Saída: `0 1 2 10 9 1 2 3`. |
| **05** | Estrutura Switch | `05_switch_case.java` | `05_switch_case.c` | ✅ **PASSOU** | `switch`, múltiplos `case`, `break` e `default`. Saída: `200`. |
| **06** | Métodos e Funções | `06_metodos_e_funcoes.java` | `06_metodos_e_funcoes.c` | ✅ **PASSOU** | Métodos estáticos com múltiplos parâmetros, retorno e recursão. Saída: `40`, `120`. |
| **07** | Atributos Globais | `07_atributos_globais.java` | `07_atributos_globais.c` | ✅ **PASSOU** | Algoritmo de duas passagens emitiu variáveis globais no topo do C. Saída: `Resultado`, `12`. |
| **08** | Escopos e Blocos | `08_escopos_e_blocos.java` | `08_escopos_e_blocos.c` | ✅ **PASSOU** | Blocos aninhados `{ ... }` e sombreamento de variáveis (*shadowing*). Saída: `10`, `20`, `10`. |
| **09** | Entrada com Scanner | `09_scanner_entrada.java` | `09_scanner_entrada.c` | ✅ **PASSOU** | Mapeamento de `Scanner` para `scanf("%d", &numero)`. Saída conforme entrada. |
| **10** | Anomalia String + Int | `10_anomalia_soma_string.java` | `10_anomalia_soma_string.c` | ✅ **PASSOU** | Demonstra aritmética de ponteiros em C (`"abcdef" + 2` $\rightarrow$ `"cdef"`). |
| **11** | Declaração de Vetor | `11_limite_declaracao_array.java` | *(Nenhum)* | ❌ **LIMITAÇÃO** | **Rejeitado no Sintático:** A gramática não possui regra para colchetes em variáveis locais (`int[] v`). |
| **12** | Igualdade Booleana | `12_limite_igualdade_booleana.java` | `12_limite_igualdade_booleana.c` | ✅ **PASSOU** | Validação semântica de igualdade (`boolean b = (3 == 3)`). Saída: `true`. |
| **13** | Importação de Pacotes | `13_limite_import.java` | *(Nenhum)* | ❌ **LIMITAÇÃO** | **Rejeitado no Sintático:** A palavra-chave `import` não faz parte da gramática. |
| **14** | Ordem de Métodos | `14_metodo_declarado_depois.java` | `14_metodo_declarado_depois.c` | ✅ **PASSOU** | Método chamado na `main` antes de sua definição no final. Saída: `20` (com aviso de protótipo no GCC). |

---

## 3. Detalhamento dos Cenários que Não Passam (Limitações de Escopo)

### Teste 11: `int[] vetor = new int[5];`
* **Etapa onde falha:** Análise Sintática (Bison).
* **Mensagem:** `Erro sintatico: na linha 10: syntax error`.
* **Causa:** A gramática do projeto aceita colchetes `[]` na lista de parâmetros de métodos (`String[] args`), na expressão `new int[N]` e no acesso indexado `v[i]`, mas não possui a produção de colchetes na regra `declaracao_local` para variáveis locais ou de classe.

### Teste 13: `import java.util.Scanner;`
* **Etapa onde falha:** Análise Sintática (Bison / Flex).
* **Mensagem:** `Erro sintatico: na linha 8: syntax error`.
* **Causa:** O compilador trata classes como `Scanner` e objetos como `System.out` como primitivas embutidas diretamente na gramática da linguagem. A construção de importação de pacotes (`import`) está formalmente fora do escopo da disciplina.
