#!/usr/bin/env bash

set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PARSER="$ROOT_DIR/build/parser"
TEST_DIR="$ROOT_DIR/tests/ast"
passou=0
falhou=0

if [[ ! -x "$PARSER" ]]; then
    echo "Erro: parser nao encontrado em $PARSER. Execute 'make parser' primeiro." >&2
    exit 2
fi

executar_positivo() {
    local arquivo="$1"
    local saida status

    saida="$("$PARSER" < "$arquivo" 2>&1)"
    status=$?

    if [[ $status -eq 0 && "$saida" == *"Analise sintatica concluida com sucesso."* ]]; then
        printf '  [PASS] %s foi aceito\n' "$(basename "$arquivo")"
        ((passou += 1))
    else
        printf '  [FAIL] %s deveria ser aceito (status %d)\n' "$(basename "$arquivo")" "$status"
        printf '         Saida: %s\n' "${saida:-<sem saida>}"
        ((falhou += 1))
    fi
}

executar_negativo() {
    local arquivo="$1"
    local linha_esperada="$2"
    local saida status

    saida="$("$PARSER" < "$arquivo" 2>&1)"
    status=$?

    if [[ $status -ne 0 && "$saida" == *"Erro sintatico"* ]] \
        && grep -Eiq "linha[[:space:]:]+${linha_esperada}([^0-9]|$)" <<< "$saida"; then
        printf '  [PASS] %s foi rejeitado com erro na linha %s\n' "$(basename "$arquivo")" "$linha_esperada"
        ((passou += 1))
    else
        printf '  [FAIL] %s deveria ser rejeitado com erro informativo na linha %s (status %d)\n' \
            "$(basename "$arquivo")" "$linha_esperada" "$status"
        printf '         Saida: %s\n' "${saida:-<sem saida>}"
        ((falhou += 1))
    fi
}

executar_switch_positivo() {
    local arquivo="$1"
    local esperado="$2"
    local ausente="${3:-}"
    local casos_esperados="${4:-1}"
    local saida status casos_encontrados

    saida="$("$PARSER" < "$arquivo" 2>&1)"
    status=$?
    casos_encontrados="$(grep -Fc 'case: NO_CASE' <<< "$saida" || true)"

    if [[ $status -eq 0 && "$saida" == *"Analise sintatica concluida com sucesso."* \
        && "$saida" == *"NO_SWITCH"* && "$saida" == *"NO_CASES"* \
        && "$saida" == *"NO_CASE"* && "$saida" == *"NO_BREAK"* \
        && "$saida" == *"$esperado"* \
        && "$casos_encontrados" -eq "$casos_esperados" ]] \
        && { [[ -z "$ausente" ]] || [[ "$saida" != *"$ausente"* ]]; }; then
        printf '  [PASS] %s aceito e AST verificada\n' "$(basename "$arquivo")"
        ((passou += 1))
    else
        printf '  [FAIL] %s deveria produzir a AST esperada (status %d)\n' \
            "$(basename "$arquivo")" "$status"
        printf '         Saida: %s\n' "${saida:-<sem saida>}"
        ((falhou += 1))
    fi
}

echo "=== Testes de comandos if ==="
executar_positivo "$TEST_DIR/valido_if_simples.java"
executar_positivo "$TEST_DIR/valido_if_else_aninhado.java"
executar_positivo "$TEST_DIR/valido_if_sem_blocos.java"
executar_positivo "$TEST_DIR/valido_if_condicao_composta.java"
executar_positivo "$TEST_DIR/valido_if_blocos_vazios.java"
executar_negativo "$TEST_DIR/invalido_if_sem_condicao.java" 3
executar_negativo "$TEST_DIR/invalido_if_sem_corpo.java" 4
executar_negativo "$TEST_DIR/invalido_if_condicao_incompleta.java" 3
executar_negativo "$TEST_DIR/invalido_else_sem_if.java" 3
executar_negativo "$TEST_DIR/invalido_if_else_duplicado.java" 7

echo "=== Testes de laços ==="
executar_positivo "$TEST_DIR/valido_laco_while.java"
executar_positivo "$TEST_DIR/valido_laco_do_while.java"
executar_positivo "$TEST_DIR/valido_laco_for_completo.java"
executar_positivo "$TEST_DIR/valido_laco_for_vazio.java"
executar_negativo "$TEST_DIR/invalido_do_while_sem_ponto_virgula.java" 5

echo "=== Testes de switch ==="
executar_switch_positivo "$TEST_DIR/valido_switch_com_cases_e_default.java" \
    "default: NO_DEFAULT" "" 2
executar_switch_positivo "$TEST_DIR/valido_switch_sem_default.java" \
    "default: <vazio>" "default: NO_DEFAULT" 1
executar_negativo "$TEST_DIR/invalido_switch_sem_chaves.java" 4

printf 'Resumo: %d passou, %d falhou.\n' "$passou" "$falhou"
(( falhou == 0 ))
