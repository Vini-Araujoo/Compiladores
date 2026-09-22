#!/usr/bin/env bash

set -u

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PARSER="$ROOT_DIR/build/parser"
ISSUE_INPUT="${1:-issue_4}"

if [[ "$ISSUE_INPUT" =~ ^[0-9]+$ ]]; then
    ISSUE_DIR="issue_$ISSUE_INPUT"
else
    ISSUE_DIR="$ISSUE_INPUT"
fi

TEST_DIR="$ROOT_DIR/tests/parser/$ISSUE_DIR"

if [[ ! -x "$PARSER" ]]; then
    echo "Erro: parser não encontrado em $PARSER. Execute 'make parser' primeiro." >&2
    exit 2
fi

if [[ ! -d "$TEST_DIR" ]]; then
    echo "Erro: diretório de testes não encontrado: $TEST_DIR" >&2
    exit 2
fi

shopt -s nullglob
POSITIVOS=("$TEST_DIR"/valido_*.java)
NEGATIVOS=("$TEST_DIR"/invalido_*.java)

if (( ${#POSITIVOS[@]} == 0 && ${#NEGATIVOS[@]} == 0 )); then
    echo "Erro: nenhum teste encontrado em $TEST_DIR" >&2
    exit 2
fi

passou=0
falhou=0

executar_teste() {
    local arquivo="$1"
    local esperado="$2"
    local saida
    local status

    saida="$("$PARSER" < "$arquivo" 2>&1)"
    status=$?

    if [[ "$esperado" == "sucesso" && $status -eq 0 ]]; then
        printf '[PASS] %s\n' "$(basename "$arquivo")"
        ((passou++))
    elif [[ "$esperado" == "erro" && $status -ne 0 && "$saida" == *"Erro sintatico:"* ]]; then
        printf '[PASS] %s\n' "$(basename "$arquivo")"
        ((passou++))
    else
        printf '[FAIL] %s (esperado: %s; status: %d)\n' \
            "$(basename "$arquivo")" "$esperado" "$status"
        printf '       Saída: %s\n' "${saida:-<sem saída>}"
        ((falhou++))
    fi
}

for arquivo in "${POSITIVOS[@]}"; do
    executar_teste "$arquivo" sucesso
done

for arquivo in "${NEGATIVOS[@]}"; do
    executar_teste "$arquivo" erro
done

printf '\nResumo: %d passou, %d falhou.\n' "$passou" "$falhou"

if (( falhou > 0 )); then
    exit 1
fi
