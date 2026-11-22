#!/usr/bin/env bash

set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
TEST_DIR="$ROOT_DIR/tests/codegen"
CC_C="${CC:-gcc}"
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

DRIVER="$(resolve_bin "$BUILD_DIR/teste_codegen_expressoes")"

if [[ ! -x "$DRIVER" ]]; then
    echo "Erro: $DRIVER nao encontrado. Execute 'make test-codegen' (ele compila o programa de teste)." >&2
    exit 2
fi

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

executar_codegen_positivo() {
    local nome="$1"
    local arquivo="$TEST_DIR/$nome.java"
    local esperado_c="$TEST_DIR/esperado/$nome.c.txt"
    local esperado_saida="$TEST_DIR/esperado/$nome.saida.txt"
    local fonte_c="$TMP_DIR/$nome.c"
    local erros="$TMP_DIR/$nome.err"
    local binario="$TMP_DIR/$nome.bin"
    local status corpo saida_exec compilacao detalhe_execucao

    "$DRIVER" < "$arquivo" > "$fonte_c" 2> "$erros"
    status=$?
    if [[ $status -ne 0 || -s "$erros" ]]; then
        printf '  [FAIL] %s.java: o gerador falhou ou emitiu erros (status %d)\n' "$nome" "$status"
        printf '         Saida: %s\n' "$(head -c 300 "$erros")"
        ((falhou += 1))
        return
    fi

    corpo="$(sed -n '/^int main(void) {/,$p' "$fonte_c")"
    if [[ "$corpo" != "$(cat "$esperado_c")" ]]; then
        printf '  [FAIL] %s.java: o C gerado difere de %s\n' "$nome" "esperado/$nome.c.txt"
        diff <(printf '%s\n' "$corpo") "$esperado_c" | head -20 | sed 's/^/         /'
        ((falhou += 1))
        return
    fi

    compilacao="$("$CC_C" -Wall -Wextra -Werror "$fonte_c" -o "$binario" 2>&1)"
    status=$?
    if [[ $status -ne 0 ]]; then
        printf '  [FAIL] %s.java: o C gerado nao compila limpo com -Wall -Wextra -Werror\n' "$nome"
        printf '%s\n' "$compilacao" | head -10 | sed 's/^/         /'
        ((falhou += 1))
        return
    fi

    if [[ -f "$esperado_saida" ]]; then
        saida_exec="$("$(resolve_bin "$binario")" 2>&1)"
        saida_exec="${saida_exec//$'\r'/}"
        if [[ "$saida_exec" != "$(tr -d '\r' < "$esperado_saida")" ]]; then
            printf '  [FAIL] %s.java: a execucao do C difere de %s\n' "$nome" "esperado/$nome.saida.txt"
            diff <(printf '%s\n' "$saida_exec") <(tr -d '\r' < "$esperado_saida") | head -20 | sed 's/^/         /'
            ((falhou += 1))
            return
        fi
        detalhe_execucao="e produziu a mesma saida que o Java"
    else
        detalhe_execucao="(sem comparacao de execucao)"
    fi

    printf '  [PASS] %s.java gerou o C esperado, compilou sem warnings %s\n' "$nome" "$detalhe_execucao"
    ((passou += 1))
}

echo "=== Testes de geracao de codigo: expressoes aritmeticas e literais (Issue 10.7) ==="
executar_codegen_positivo "valido_expr_aritmetica_simples"
executar_codegen_positivo "valido_expr_aritmetica_composta"
executar_codegen_positivo "valido_expr_todos_literais"

echo "=== Testes de geracao de codigo: operadores relacionais, logicos e unarios (Issue 10.8) ==="
executar_codegen_positivo "valido_expr_relacionais"
executar_codegen_positivo "valido_expr_logicos_curto_circuito"
executar_codegen_positivo "valido_expr_unarios_pre_pos"

printf 'Resumo Geracao de Codigo: %d passou, %d falhou.\n' "$passou" "$falhou"
(( falhou == 0 ))