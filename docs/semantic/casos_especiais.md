# Análise Estática de Casos Especiais

Documentação formal sobre a interceptação estática de erros matemáticos e limites semânticos da árvore abstrata sintática (Issue 9.5).

---

## 1. Visão Geral
Este documento detalha as verificações de análise estática realizadas na árvore sintática abstrata (AST) durante a fase de inferência de tipos. O objetivo é interceptar em tempo de compilação erros grosseiros e previsíveis, como divisões literais por zero.

## 2. Divisão por Zero
A divisão por zero (`/` ou `%`) é uma operação matemática indefinida. A análise semântica identifica este problema avaliando o operando direito de operações binárias.

- **Divisão literal por zero:** O compilador emite um erro semântico de divisão por zero caso o filho direito do nó da operação seja um `NO_LITERAL_INT` de valor `0`, ou `NO_LITERAL_DOUBLE` com `0.0`. A compilação é abortada.
- **Divisão por variável:** Se o divisor for um identificador (variável), a validação estática ignora o nó. Em Java, a divisão inteira por zero avaliada dinamicamente lança `ArithmeticException`, logo, a resolução deste caso pertence estritamente ao tempo de execução (runtime).

## 3. Validação de Faixa de Valores (Pendente)
A especificação demanda validar estouro de literais para tipos `byte` e `short`. Contudo, literais isolados na AST são sempre nativamente tipados como inteiros. 
- **Limitação Atual:** A validação de estouro exige o conhecimento prévio do tipo da variável de destino (ex: `byte b = 200;`), o que é responsabilidade do nó de declaração e da Tabela de Símbolos.
- **Decisão Técnica:** Esta verificação fica pendente e será implementada integralmente após a integração da Issue 8.2 (Tabela de Escopos), momento em que o compilador conseguirá comparar o valor do literal contra o tipo formalmente declarado da variável.

## 4. Casos de Teste
- **Testes Positivos:**
  - `tests/semantic/valido_divisao_por_variavel.java`: Assegura que variáveis não disparam bloqueio na análise estática.
- **Testes Negativos:**
  - `tests/semantic/invalido_divisao_literal_zero.java`: Rejeita divisão inteira por zero na compilação.
  - `tests/semantic/invalido_modulo_literal_zero.java`: Rejeita operação de módulo duplo por zero na compilação.