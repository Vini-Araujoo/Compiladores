# Tradução de controle de fluxo para C

Este documento descreve como o gerador traduz os nós de controle de fluxo da AST Java para C. A implementação está em `src/codegen/codegen_fluxo.c`.

| Estrutura Java | Forma gerada em C | Observações |
|---|---|---|
| `if (condicao) { ... }` | `if (condicao) { ... }` | A condição é emitida por `emitirExpressao`; o corpo é percorrido no nível de indentação seguinte. |
| `if (...) { ... } else { ... }` | `if (...) { ... } else { ... }` | Um `else if` é emitido na mesma linha como `} else if (...) {`. |
| `while (condicao) { ... }` | `while (condicao) { ... }` | O corpo é percorrido como uma lista de comandos dentro do bloco. |
| `do { ... } while (condicao);` | `do { ... } while (condicao);` | O bloco executa ao menos uma vez, como em Java. |
| `for (int i = 0; condicao; i++) { ... }` | `for (int i = 0; condicao; i++) { ... }` | A AST guarda inicialização, condição, incremento e corpo em `filhos[0..3]`. A inicialização e o incremento são emitidos inline. |
| `switch (valor) { case 1: ... break; default: ... }` | `switch (valor) { case 1: ... break; default: ... break; }` | Os nós `NO_CASE` e `NO_DEFAULT` guardam blocos de comandos. O gerador acrescenta `break;` quando o bloco não contém um `NO_BREAK` direto. |
| `break;` | `break;` | Encaminhado pelo dispatcher de fluxo para `emitirBreak`. |
| `return expressao;` | `return expressao;` | A expressão opcional é emitida por `emitirExpressao`. |
| `return;` | `return;` | Usado em métodos sem valor de retorno. |
| `continue;` | `continue;` | Encaminhado pelo dispatcher de fluxo. |

## Exemplo

Java:

```java
int total = 0;
for (int i = 0; i < 3; i++) {
    total += i;
}
```

C gerado:

```c
int total = 0;
for (int i = 0; (i < 3); (i++)) {
    total += i;
}
```
