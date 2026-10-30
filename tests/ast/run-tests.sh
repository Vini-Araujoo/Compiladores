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

executar_classe_positiva() {
    local arquivo="$1"
    local classe="$2"
    local saida status

    saida="$("$PARSER" < "$arquivo" 2>&1)"
    status=$?

    if [[ $status -eq 0 && "$saida" == *"NO_CLASSE (${classe})"* \
        && "$saida" == *"Analise sintatica concluida com sucesso."* ]]; then
        printf '  [PASS] %s foi aceito e a AST da classe foi verificada\n' "$(basename "$arquivo")"
        ((passou += 1))
    else
        printf '  [FAIL] %s deveria produzir NO_CLASSE (%s) (status %d)\n' \
            "$(basename "$arquivo")" "$classe" "$status"
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

executar_metodo_positivo() {
    local arquivo="$1"
    local metodo="$2"
    local saida status

    saida="$("$PARSER" < "$arquivo" 2>&1)"
    status=$?

    if [[ $status -eq 0 && "$saida" == *"NO_METODO (${metodo})"* \
        && "$saida" == *"NO_PARAMETROS"* \
        && "$saida" == *"NO_BLOCO"* \
        && "$saida" == *"Analise sintatica concluida com sucesso."* ]]; then
        printf '  [PASS] %s aceito e AST de metodo verificada\n' "$(basename "$arquivo")"
        ((passou += 1))
    else
        printf '  [FAIL] %s deveria produzir NO_METODO (%s) (status %d)\n' \
            "$(basename "$arquivo")" "$metodo" "$status"
        printf '         Saida: %s\n' "${saida:-<sem saida>}"
        ((falhou += 1))
    fi
}

executar_arvore_positiva() {
    local arquivo="$1"
    local esperado="$2"
    local saida status

    saida="$("$PARSER" < "$arquivo" 2>&1)"
    status=$?

    if [[ $status -eq 0 && "$saida" == "$(cat "$esperado")" ]]; then
        printf '  [PASS] %s aceito e arvore identica a %s\n' \
            "$(basename "$arquivo")" "$(basename "$esperado")"
        ((passou += 1))
    else
        printf '  [FAIL] %s deveria produzir a arvore de %s (status %d)\n' \
            "$(basename "$arquivo")" "$(basename "$esperado")" "$status"
        diff <(printf '%s\n' "$saida") "$esperado" | head -20 | sed 's/^/         /'
        ((falhou += 1))
    fi
}

echo "=== Testes de declaracao de classe ==="
executar_classe_positiva "$TEST_DIR/valido_classe_simples.java" "Simples"
executar_classe_positiva "$TEST_DIR/valido_classe_com_modificadores.java" "ComModificadores"
executar_negativo "$TEST_DIR/invalido_classe_sem_nome.java" 1

echo "=== Testes de declaracao de metodos ==="
executar_metodo_positivo "$TEST_DIR/valido_metodo_main.java" "main"
executar_metodo_positivo "$TEST_DIR/valido_metodo_multiplos_params.java" "calcular"
executar_negativo "$TEST_DIR/invalido_metodo_sem_parenteses.java" 2

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

echo "=== Testes de arrays ==="
executar_positivo "$TEST_DIR/valido_alocacao_array.java"
executar_positivo "$TEST_DIR/valido_atribuicao_em_indice_array.java"
executar_negativo "$TEST_DIR/invalido_array_sem_tamanho.java" 3

echo "=== Testes de expressoes aritmeticas ==="
executar_arvore_positiva "$TEST_DIR/valido_precedencia_aritmetica.java" \
    "$TEST_DIR/esperado/valido_precedencia_aritmetica.txt"
executar_arvore_positiva "$TEST_DIR/valido_expressao_com_parenteses.java" \
    "$TEST_DIR/esperado/valido_expressao_com_parenteses.txt"
executar_negativo "$TEST_DIR/invalido_operador_sem_operando.java" 3

echo "=== Testes de operadores relacionais, logicos e unarios ==="
executar_arvore_positiva "$TEST_DIR/valido_expressoes_logicas_complexas.java" \
    "$TEST_DIR/esperado/valido_expressoes_logicas_complexas.txt"
executar_arvore_positiva "$TEST_DIR/valido_operadores_incremento.java" \
    "$TEST_DIR/esperado/valido_operadores_incremento.txt"
executar_negativo "$TEST_DIR/invalido_operador_logico_duplicado.java" 3

echo "=== Testes de declaracoes de variaveis e atribuicoes ==="
executar_arvore_positiva "$TEST_DIR/valido_declaracao_multipla.java" \
    "$TEST_DIR/esperado/valido_declaracao_multipla.txt"
executar_arvore_positiva "$TEST_DIR/valido_atribuicao_simples.java" \
    "$TEST_DIR/esperado/valido_atribuicao_simples.txt"
executar_negativo "$TEST_DIR/invalido_declaracao_sem_tipo.java" 3

printf 'Resumo: %d passou, %d falhou.\n' "$passou" "$falhou"
(( falhou == 0 ))