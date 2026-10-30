#!/usr/bin/env bash

set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SEMANTIC="$ROOT_DIR/build/semantic"
TEST_DIR="$ROOT_DIR/tests/semantic"
passou=0
falhou=0

if [[ ! -x "$SEMANTIC" ]]; then
    echo "Erro: analisador semantico nao encontrado em $SEMANTIC. Execute 'make semantic' primeiro." >&2
    exit 2
fi

executar_arvore_positiva() {
    local arquivo="$1"
    local esperado="$2"
    local saida status

    saida="$("$SEMANTIC" < "$arquivo" 2>&1)"
    status=$?

    if [[ $status -eq 0 && "$saida" == "$(cat "$esperado")" ]]; then
        printf '  [PASS] %s aceito e arvore tipada identica a %s\n' \
            "$(basename "$arquivo")" "$(basename "$esperado")"
        ((passou += 1))
    else
        printf '  [FAIL] %s deveria produzir a arvore tipada de %s (status %d)\n' \
            "$(basename "$arquivo")" "$(basename "$esperado")" "$status"
        diff <(printf '%s\n' "$saida") "$esperado" | head -20 | sed 's/^/         /'
        ((falhou += 1))
    fi
}

executar_negativo() {
    local arquivo="$1"
    local linha_esperada="$2"
    local saida status

    saida="$("$SEMANTIC" < "$arquivo" 2>&1)"
    status=$?

    if [[ $status -ne 0 && "$saida" == *"Erro semantico"* ]] \
        && grep -Eiq "linha[[:space:]:]+${linha_esperada}([^0-9]|$)" <<< "$saida"; then
        printf '  [PASS] %s foi rejeitado com erro na linha %s\n' "$(basename "$arquivo")" "$linha_esperada"
        ((passou += 1))
    else
        printf '  [FAIL] %s deveria ser rejeitado com erro semantico na linha %s (status %d)\n' \
            "$(basename "$arquivo")" "$linha_esperada" "$status"
        printf '         Saida: %s\n' "${saida:-<sem saida>}"
        ((falhou += 1))
    fi
}

echo "=== Testes de inferencia de tipos ==="
executar_arvore_positiva "$TEST_DIR/valido_inferencia_mista_int_double.java" \
    "$TEST_DIR/esperado/valido_inferencia_mista_int_double.txt"
executar_negativo "$TEST_DIR/invalido_tipo_indeterminado.java" 3

printf 'Resumo: %d passou, %d falhou.\n' "$passou" "$falhou"
(( falhou == 0 ))