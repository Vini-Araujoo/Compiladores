#!/usr/bin/env bash

set -uo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
passou=0
falhou=0

echo "=== Executando testes unitarios da Tabela de Simbolos e Escopos (Issue 8.1) ==="

# 1. Teste positivo de pilha de escopos e busca ascendente
if [[ -x "$BUILD_DIR/teste_pilha_escopos" ]]; then
    saida="$("$BUILD_DIR/teste_pilha_escopos" 2>&1)"
    status=$?
    if [[ $status -eq 0 && "$saida" == *"teste_pilha_escopos passou com sucesso!"* ]]; then
        printf '  [PASS] teste_pilha_escopos aprovado com status %d\n' "$status"
        ((passou += 1))
    else
        printf '  [FAIL] teste_pilha_escopos falhou (status %d)\n' "$status"
        printf '         Saida: %s\n' "$saida"
        ((falhou += 1))
    fi
else
    echo "Erro: binario teste_pilha_escopos nao encontrado em $BUILD_DIR." >&2
    exit 2
fi

# 2. Teste negativo de busca apos fechamento e rejeicao de redeclaracao
if [[ -x "$BUILD_DIR/teste_busca_apos_fechamento_escopo" ]]; then
    saida="$("$BUILD_DIR/teste_busca_apos_fechamento_escopo" 2>&1)"
    status=$?
    if [[ $status -eq 0 && "$saida" == *"teste_busca_apos_fechamento_escopo passou com sucesso!"* \
        && "$saida" == *"Erro semantico na linha 12"* ]]; then
        printf '  [PASS] teste_busca_apos_fechamento_escopo aprovado com rejeicao informativa validada na linha 12\n'
        ((passou += 1))
    else
        printf '  [FAIL] teste_busca_apos_fechamento_escopo falhou (status %d)\n' "$status"
        printf '         Saida: %s\n' "$saida"
        ((falhou += 1))
    fi
else
    echo "Erro: binario teste_busca_apos_fechamento_escopo nao encontrado em $BUILD_DIR." >&2
    exit 2
fi

echo
printf 'Resumo Tabela de Simbolos: %d passou, %d falhou.\n' "$passou" "$falhou"
(( falhou == 0 ))
