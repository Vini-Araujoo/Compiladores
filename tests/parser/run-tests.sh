#!/usr/bin/env bash

set -u

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PARSER="$ROOT_DIR/build/parser"
ISSUE_INPUT="${1:-all}"

if [[ ! -x "$PARSER" ]]; then
    echo "Erro: parser nao encontrado em $PARSER. Execute 'make parser' primeiro." >&2
    exit 2
fi

if [[ "$ISSUE_INPUT" == "all" || -z "$ISSUE_INPUT" ]]; then
    DIRS=("issue_1" "issue_2" "issue_3" "issue_4" "issue_5")
elif [[ "$ISSUE_INPUT" =~ ^[0-9]+$ ]]; then
    DIRS=("issue_$ISSUE_INPUT")
else
    DIRS=("$ISSUE_INPUT")
fi

total_passou=0
total_falhou=0

executar_teste() {
    local arquivo="$1"
    local esperado="$2"
    local saida
    local status

    saida="$("$PARSER" < "$arquivo" 2>&1)"
    status=$?

    if [[ "$esperado" == "sucesso" && $status -eq 0 ]]; then
        printf '  [PASS] %s\n' "$(basename "$arquivo")"
        ((total_passou++))
    elif [[ "$esperado" == "erro" && $status -ne 0 && "$saida" == *"Erro sintatico:"* ]]; then
        printf '  [PASS] %s\n' "$(basename "$arquivo")"
        ((total_passou++))
    else
        printf '  [FAIL] %s (esperado: %s; status: %d)\n' \
            "$(basename "$arquivo")" "$esperado" "$status"
        printf '         Saida: %s\n' "${saida:-<sem saida>}"
        ((total_falhou++))
    fi
}

shopt -s nullglob

for dir_name in "${DIRS[@]}"; do
    if [[ "$dir_name" = /* ]]; then
        TEST_DIR="$dir_name"
    elif [[ -d "$ROOT_DIR/tests/parser/$dir_name" ]]; then
        TEST_DIR="$ROOT_DIR/tests/parser/$dir_name"
    elif [[ -d "$dir_name" ]]; then
        TEST_DIR="$dir_name"
    else
        echo "Erro: diretorio de testes nao encontrado: $ROOT_DIR/tests/parser/$dir_name" >&2
        exit 2
    fi

    echo "=== Executando testes: $(basename "$TEST_DIR") ==="

    POSITIVOS=("$TEST_DIR"/valido_*.java)
    NEGATIVOS=("$TEST_DIR"/invalido_*.java)

    if (( ${#POSITIVOS[@]} == 0 && ${#NEGATIVOS[@]} == 0 )); then
        echo "  (Nenhum teste encontrado)"
        echo
        continue
    fi

    for arquivo in "${POSITIVOS[@]}"; do
        executar_teste "$arquivo" sucesso
    done

    for arquivo in "${NEGATIVOS[@]}"; do
        executar_teste "$arquivo" erro
    done
    echo
done

printf 'Resumo Geral: %d passou, %d falhou.\n' "$total_passou" "$total_falhou"

if (( total_falhou > 0 )); then
    exit 1
fi
