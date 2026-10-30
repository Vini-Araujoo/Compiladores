# Regras de Validação de Operadores

Documentação das regras de validação semântica de compatibilidade de tipos para operadores unários e binários (Issue 9.4).

---

## 1. Visão Geral
Este documento especifica as regras de validação semântica aplicadas aos operadores da linguagem, garantindo que operandos incompatíveis sejam rejeitados em tempo de compilação.

## 2. Regras de Compatibilidade

### 2.1 Operadores Aritméticos (`+`, `-`, `*`, `/`, `%`)
- **Permitidos:** Tipos numéricos (`byte`, `short`, `int`, `long`, `float`, `double`, `char`).
- **Proibidos:** Tipos `boolean`. Operações como `true + 1` geram erro semântico.
- **Unários (`+`, `-`, `++`, `--`):** Exigem operando estritamente numérico.

### 2.2 Operadores Lógicos (`&&`, `||`)
- **Permitidos:** Apenas tipos `boolean`.
- **Proibidos:** Tipos numéricos. Uma operação como `5 && 10` gera erro semântico.

### 2.3 Operadores Relacionais (`<`, `<=`, `>`, `>=`)
- **Permitidos:** Apenas tipos numéricos.
- **Proibidos:** Tipos `boolean`. Comparações do tipo `true > false` não são suportadas.

## 3. Comportamento e Retorno
Ao detectar incompatibilidade, a etapa de inferência de tipos (via `inferirOperacaoBinaria` ou `inferirOperacaoUnaria`) emite a notificação exata da linha em `stderr` e retorna o fallback `TIPO_INDEFINIDO` para anular a validação dos nós ascendentes e cancelar a geração do binário.