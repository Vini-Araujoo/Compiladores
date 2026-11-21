#!/usr/bin/env bash

set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PARSER="$ROOT_DIR/build/parser"
TEST_DIR="$ROOT_DIR/tests/codegen"
passou=0
falhou=0

if [[ ! -x "$PARSER" ]]; then
    echo "Erro: parser nao encontrado em $PARSER. Execute 'make parser' primeiro." >&2
    exit 2
fi

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

printf 'Resumo: %d passou, %d falhou.\n' "$passou" "$falhou"
(( falhou == 0 ))