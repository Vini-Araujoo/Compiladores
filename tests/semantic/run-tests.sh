#!/usr/bin/env bash

set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
SEMANTIC_DIR="$ROOT_DIR/tests/semantic"
passou=0
falhou=0

echo "=== Executando testes unitarios da Tabela de Simbolos e Escopos (Issue 8.1) ==="

# 1. Teste positivo de pilha de escopos e busca ascendente
if [[ -x "$BUILD_DIR/teste_pilha_escopos" ]]; then
    saida="$("$BUILD_DIR/teste_pilha_escopos" 2>&1)"
    status=$?
    if [[ $status -eq 0 && "$saida" == *"teste_pilha_escopos passou com sucesso!"* ]]; then
        printf '  [PASS] teste_pilha_escopos aprovado com status %d\n' "$status"
        ((passou += 1))
    else
        printf '  [FAIL] teste_pilha_escopos falhou (status %d)\n' "$status"
        printf '         Saida: %s\n' "$saida"
        ((falhou += 1))
    fi
else
    echo "Erro: binario teste_pilha_escopos nao encontrado em $BUILD_DIR." >&2
    exit 2
fi

# 2. Teste negativo de busca apos fechamento e rejeicao de redeclaracao
if [[ -x "$BUILD_DIR/teste_busca_apos_fechamento_escopo" ]]; then
    saida="$("$BUILD_DIR/teste_busca_apos_fechamento_escopo" 2>&1)"
    status=$?
    if [[ $status -eq 0 && "$saida" == *"teste_busca_apos_fechamento_escopo passou com sucesso!"* \
        && "$saida" == *"Erro semantico na linha 12"* ]]; then
        printf '  [PASS] teste_busca_apos_fechamento_escopo aprovado com rejeicao informativa validada na linha 12\n'
        ((passou += 1))
    else
        printf '  [FAIL] teste_busca_apos_fechamento_escopo falhou (status %d)\n' "$status"
        printf '         Saida: %s\n' "$saida"
        ((falhou += 1))
    fi
else
    echo "Erro: binario teste_busca_apos_fechamento_escopo nao encontrado em $BUILD_DIR." >&2
    exit 2
fi

echo
echo "=== Executando testes de Resolucao de Variaveis e Escopos (Issue 8.3) ==="

# 3. Teste unitario em C de resolucao de escopos
if [[ -x "$BUILD_DIR/teste_escopos_unitario" ]]; then
    saida="$("$BUILD_DIR/teste_escopos_unitario" 2>&1)"
    status=$?
    if [[ $status -eq 0 && "$saida" == *"teste_escopos_unitario passou com sucesso!"* ]]; then
        printf '  [PASS] teste_escopos_unitario aprovado com status %d\n' "$status"
        ((passou += 1))
    else
        printf '  [FAIL] teste_escopos_unitario falhou (status %d)\n' "$status"
        printf '         Saida: %s\n' "$saida"
        ((falhou += 1))
    fi
else
    echo "Erro: binario teste_escopos_unitario nao encontrado em $BUILD_DIR." >&2
    exit 2
fi

# 4. Teste positivo com o parser: valido_escopos_aninhados_permitidos.java
if [[ -x "$BUILD_DIR/parser" ]]; then
    saida="$("$BUILD_DIR/parser" --semantico < "$SEMANTIC_DIR/valido_escopos_aninhados_permitidos.java" 2>&1)"
    status=$?
    if [[ $status -eq 0 ]]; then
        printf '  [PASS] valido_escopos_aninhados_permitidos.java aprovado com status %d\n' "$status"
        ((passou += 1))
    else
        printf '  [FAIL] valido_escopos_aninhados_permitidos.java falhou (status %d)\n' "$status"
        printf '         Saida: %s\n' "$saida"
        ((falhou += 1))
    fi
else
    echo "Erro: binario parser nao encontrado em $BUILD_DIR." >&2
    exit 2
fi

# 5. Teste negativo com o parser: invalido_variavel_nao_declarada.java
if [[ -x "$BUILD_DIR/parser" ]]; then
    saida="$("$BUILD_DIR/parser" --semantico < "$SEMANTIC_DIR/invalido_variavel_nao_declarada.java" 2>&1)"
    status=$?
    if [[ $status -ne 0 && "$saida" == *"Erro semantico na linha 5"* && "$saida" == *"'b' nao declarada"* ]]; then
        printf '  [PASS] invalido_variavel_nao_declarada.java rejeitado corretamente na linha 5 com mensagem informativa\n'
        ((passou += 1))
    else
        printf '  [FAIL] invalido_variavel_nao_declarada.java nao falhou como esperado (status %d)\n' "$status"
        printf '         Saida: %s\n' "$saida"
        ((falhou += 1))
    fi
fi

# 6. Teste negativo com o parser: invalido_redeclaracao_mesmo_escopo.java
if [[ -x "$BUILD_DIR/parser" ]]; then
    saida="$("$BUILD_DIR/parser" --semantico < "$SEMANTIC_DIR/invalido_redeclaracao_mesmo_escopo.java" 2>&1)"
    status=$?
    if [[ $status -ne 0 && "$saida" == *"Erro semantico na linha 5"* && "$saida" == *"'a' ja declarada neste escopo"* ]]; then
        printf '  [PASS] invalido_redeclaracao_mesmo_escopo.java rejeitado corretamente na linha 5 com mensagem informativa\n'
        ((passou += 1))
    else
        printf '  [FAIL] invalido_redeclaracao_mesmo_escopo.java nao falhou como esperado (status %d)\n' "$status"
        printf '         Saida: %s\n' "$saida"
        ((falhou += 1))
    fi
fi

echo
printf 'Resumo Analise Semantica: %d passou, %d falhou.\n' "$passou" "$falhou"
(( falhou == 0 ))
