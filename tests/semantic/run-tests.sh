#!/usr/bin/env bash

set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
SEMANTIC_DIR="$ROOT_DIR/tests/semantic"
passou=0
falhou=0

resolve_bin() {
    local base="$1"
    if [[ -f "$base.exe" ]]; then
        echo "$base.exe"
    else
        echo "$base"
    fi
}

exec_bin() {
    local bin="$1"
    shift
    local tentativa=0
    local status=126
    local saida=""
    while (( tentativa < 5 )); do
        saida="$("$bin" "$@" 2>&1)"
        status=$?
        if [[ $status -ne 126 ]]; then
            break
        fi
        sleep 0.4
        (( tentativa++ ))
    done
    printf '%s' "$saida"
    return $status
}

BIN_TESTE_PILHA="$(resolve_bin "$BUILD_DIR/teste_pilha_escopos")"
BIN_TESTE_BUSCA="$(resolve_bin "$BUILD_DIR/teste_busca_apos_fechamento_escopo")"
BIN_TESTE_ESCOPOS="$(resolve_bin "$BUILD_DIR/teste_escopos_unitario")"
BIN_PARSER="$(resolve_bin "$BUILD_DIR/parser")"

echo "=== Executando testes unitarios da Tabela de Simbolos e Escopos (Issue 8.1) ==="

# 1. Teste positivo de pilha de escopos e busca ascendente
if [[ -x "$BIN_TESTE_PILHA" ]]; then
    saida="$(exec_bin "$BIN_TESTE_PILHA")"
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
if [[ -x "$BIN_TESTE_BUSCA" ]]; then
    saida="$(exec_bin "$BIN_TESTE_BUSCA")"
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
if [[ -x "$BIN_TESTE_ESCOPOS" ]]; then
    saida="$(exec_bin "$BIN_TESTE_ESCOPOS")"
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
if [[ -x "$BIN_PARSER" ]]; then
    saida="$(exec_bin "$BIN_PARSER" --semantico < "$SEMANTIC_DIR/valido_escopos_aninhados_permitidos.java")"
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
if [[ -x "$BIN_PARSER" ]]; then
    saida="$(exec_bin "$BIN_PARSER" --semantico < "$SEMANTIC_DIR/invalido_variavel_nao_declarada.java")"
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
if [[ -x "$BIN_PARSER" ]]; then
    saida="$(exec_bin "$BIN_PARSER" --semantico < "$SEMANTIC_DIR/invalido_redeclaracao_mesmo_escopo.java")"
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
echo "=== Executando testes de Constantes e Modificador Final (Issue 8.4) ==="

# 7. Teste positivo de constante final: valido_constante_final.java
if [[ -x "$BIN_PARSER" ]]; then
    saida="$(exec_bin "$BIN_PARSER" --semantico < "$SEMANTIC_DIR/valido_constante_final.java")"
    status=$?
    if [[ $status -eq 0 ]]; then
        printf '  [PASS] valido_constante_final.java aprovado com status %d\n' "$status"
        ((passou += 1))
    else
        printf '  [FAIL] valido_constante_final.java falhou (status %d)\n' "$status"
        printf '         Saida: %s\n' "$saida"
        ((falhou += 1))
    fi
else
    echo "Erro: binario parser nao encontrado em $BUILD_DIR." >&2
    exit 2
fi

# 8. Teste negativo de reatribuicao de constante: invalido_reatribuicao_final.java
if [[ -x "$BIN_PARSER" ]]; then
    saida="$(exec_bin "$BIN_PARSER" --semantico < "$SEMANTIC_DIR/invalido_reatribuicao_final.java")"
    status=$?
    if [[ $status -ne 0 && "$saida" == *"Erro semantico na linha 4"* && "$saida" == *"final 'MAX' nao pode ser reatribuida"* ]]; then
        printf '  [PASS] invalido_reatribuicao_final.java rejeitado corretamente na linha 4 com mensagem informativa\n'
        ((passou += 1))
    else
        printf '  [FAIL] invalido_reatribuicao_final.java nao falhou como esperado (status %d)\n' "$status"
        printf '         Saida: %s\n' "$saida"
        ((falhou += 1))
    fi
fi

# 9. Teste negativo de incremento de constante: invalido_incremento_final.java
if [[ -x "$BIN_PARSER" ]]; then
    saida="$(exec_bin "$BIN_PARSER" --semantico < "$SEMANTIC_DIR/invalido_incremento_final.java")"
    status=$?
    if [[ $status -ne 0 && "$saida" == *"Erro semantico na linha 4"* && "$saida" == *"final 'CONTADOR' nao pode ser incrementada"* ]]; then
        printf '  [PASS] invalido_incremento_final.java rejeitado corretamente na linha 4 com mensagem informativa\n'
        ((passou += 1))
    else
        printf '  [FAIL] invalido_incremento_final.java nao falhou como esperado (status %d)\n' "$status"
        printf '         Saida: %s\n' "$saida"
        ((falhou += 1))
    fi
fi

echo
printf 'Resumo Analise Semantica: %d passou, %d falhou.\n' "$passou" "$falhou"
(( falhou == 0 ))
