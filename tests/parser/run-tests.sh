# Uso:
#   bash tests/parser/run-tests.sh <numero_da_issue>
#   make test ISSUE=<numero_da_issue>

set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT_DIR"

ISSUE=${1:-}
if [ -z "$ISSUE" ]; then
    echo "Uso: $0 <numero_da_issue>" >&2
    exit 2
fi

TEST_DIR="tests/parser/issue_${ISSUE}"
BINARY=./build/parser

if [ ! -d "$TEST_DIR" ]; then
    echo "Diretorio de testes nao encontrado: $TEST_DIR" >&2
    exit 2
fi

FALHAS=0
TOTAL=0

for input in "$TEST_DIR"/valido_*.java; do
    [ -e "$input" ] || continue
    TOTAL=$((TOTAL + 1))
    name=$(basename "$input")

    if "$BINARY" < "$input" > /tmp/parser-test-out.$$ 2>&1; then
        printf 'OK:    %s (aceito como esperado)\n' "$name"
    else
        printf 'FALHA: %s deveria ser aceito, mas foi rejeitado:\n' "$name"
        cat /tmp/parser-test-out.$$
        FALHAS=$((FALHAS + 1))
    fi
    rm -f /tmp/parser-test-out.$$
done

for input in "$TEST_DIR"/invalido_*.java; do
    [ -e "$input" ] || continue
    TOTAL=$((TOTAL + 1))
    name=$(basename "$input")

    if "$BINARY" < "$input" > /tmp/parser-test-out.$$ 2>&1; then
        printf 'FALHA: %s deveria ser rejeitado, mas foi aceito:\n' "$name"
        cat /tmp/parser-test-out.$$
        FALHAS=$((FALHAS + 1))
    else
        printf 'OK:    %s (rejeitado como esperado)\n' "$name"
    fi
    rm -f /tmp/parser-test-out.$$
done

echo "---"
echo "Total: $TOTAL | Falhas: $FALHAS"

if [ "$FALHAS" -ne 0 ]; then
    exit 1
fi

exit 0
