#!/usr/bin/env bash

set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
TEST_DIR="$ROOT_DIR/tests/codegen"
PARSER="$ROOT_DIR/build/parser"
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

if [[ ! -x "$PARSER" ]]; then
    echo "Erro: parser nao encontrado em $PARSER. Execute 'make parser' primeiro." >&2
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

executar_io_positivo() {
    local arquivo="$1"
    shift
    local saida status marcador ausente=""

    saida="$("$PARSER" < "$arquivo" 2>&1)"
    status=$?

    for marcador in "$@"; do
        if ! grep -Eq -e "$marcador" <<< "$saida"; then
            ausente="$marcador"
            break
        fi
    done

    if [[ $status -eq 0 && -z "$ausente" \
        && "$saida" == *"Analise sintatica concluida com sucesso."* ]]; then
        printf '  [PASS] %s aceito e AST de I/O verificada\n' "$(basename "$arquivo")"
        ((passou += 1))
    else
        printf '  [FAIL] %s deveria produzir a AST de I/O esperada (status %d, ausente: %s)\n' \
            "$(basename "$arquivo")" "$status" "${ausente:-nenhum}"
        printf '         Saida: %s\n' "${saida:-<sem saida>}"
        ((falhou += 1))
    fi
}

echo "=== Testes de I/O do codegen (entrada aceita pelo parser) ==="
executar_io_positivo "$TEST_DIR/valido_io_scanner_numericos.java" \
    'NO_INSTANCIACAO_SCANNER' \
    'NO_LEITURA_SCANNER \(nextInt\)' 'NO_LEITURA_SCANNER \(nextDouble\)' \
    'NO_LEITURA_SCANNER \(nextLong\)' 'NO_LEITURA_SCANNER \(nextFloat\)' \
    'NO_LEITURA_SCANNER \(nextShort\)' 'NO_LEITURA_SCANNER \(nextByte\)' \
    'NO_PRINTLN$'
executar_io_positivo "$TEST_DIR/valido_io_scanner_strings.java" \
    'NO_INSTANCIACAO_SCANNER' \
    'NO_LEITURA_SCANNER \(next\)' 'NO_LEITURA_SCANNER \(nextLine\)' \
    'NO_PRINTLN$'
executar_io_positivo "$TEST_DIR/valido_io_scanner_boolean.java" \
    'NO_INSTANCIACAO_SCANNER' \
    'NO_LEITURA_SCANNER \(nextBoolean\)' \
    'NO_PRINTLN$'
executar_io_positivo "$TEST_DIR/valido_io_println_todos_tipos.java" \
    'NO_PRINTLN$' 'NO_PRINT$' \
    'NO_LITERAL_STRING \("literal"\)'

echo "=== Testes de arrays do codegen (entrada aceita pelo parser) ==="
executar_io_positivo "$TEST_DIR/valido_array_alocacao_dinamica.java" \
    'NO_INSTANCIACAO_ARRAY' \
    'NO_ATRIBUICAO'
executar_io_positivo "$TEST_DIR/valido_array_preenchimento_laco_for.java" \
    'NO_INSTANCIACAO_ARRAY' \
    'NO_FOR' \
    'NO_ACESSO_ARRAY'
executar_io_positivo "$TEST_DIR/valido_array_leitura_indices.java" \
    'NO_INSTANCIACAO_ARRAY' \
    'NO_ACESSO_ARRAY' \
    'NO_ATRIBUICAO'

printf 'Resumo Geracao de Codigo: %d passou, %d falhou.\n' "$passou" "$falhou"
(( falhou == 0 ))
