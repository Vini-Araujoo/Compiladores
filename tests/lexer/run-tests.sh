#!/bin/sh

set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT_DIR"

make lexer

TEST_DIR=tests/lexer
BINARY=./build/lexico

for input in "$TEST_DIR"/*.java; do
    name=$(basename "$input" .java)
    output=$(mktemp)
    trap 'rm -f "$output"' EXIT

    "$BINARY" < "$input" > "$output"
    diff -u "$TEST_DIR/esperado/$name.txt" "$output"
    rm -f "$output"
    trap - EXIT
    printf 'OK: %s\n' "$name"
done
