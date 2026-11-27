#!/bin/bash
# Script de validacao da Traducao Java para C (Transpilador Fonte-para-Fonte)

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$DIR/../.." && pwd)"
COMPILADOR="$ROOT_DIR/build/compilador"
TEMP_DIR="$DIR/tmp_build"

if [ ! -f "$COMPILADOR" ]; then
    echo "Erro: Executável '$COMPILADOR' não encontrado. Execute 'make compilador' antes."
    exit 1
fi

mkdir -p "$TEMP_DIR"

passaram=0
falharam=0
total=0

echo "================================================================="
echo "   BATERIA DE TESTES DE TRADUCAO JAVA PARA C (TRANSPILADOR)"
echo "================================================================="

for java_file in "$DIR"/*.java; do
    [ -e "$java_file" ] || continue
    total=$((total + 1))
    nome_arquivo=$(basename "$java_file")
    base="${nome_arquivo%.java}"
    c_saida="$TEMP_DIR/${base}.c"
    bin_saida="$TEMP_DIR/${base}.out"

    # 1. Compilação do Java para C
    "$COMPILADOR" "$java_file" > "$c_saida" 2> "$TEMP_DIR/${base}.comp.err"
    status_comp=$?

    if [ $status_comp -eq 0 ] && [ -s "$c_saida" ]; then
        # 2. Compilação do C gerado com GCC
        gcc -Wall -Wextra -o "$bin_saida" "$c_saida" > "$TEMP_DIR/${base}.gcc.err" 2>&1
        status_gcc=$?

        if [ $status_gcc -eq 0 ]; then
            # 3. Execução do binário
            echo "42" | "$bin_saida" > "$TEMP_DIR/${base}.run.out" 2>&1
            status_run=$?
            if [ $status_run -eq 0 ]; then
                echo "  [PASS] $nome_arquivo -> Codigo C gerado e validado com sucesso."
                passaram=$((passaram + 1))
            else
                echo "  [FAIL-RUN] $nome_arquivo -> Falha na execucao de teste do C gerado (exit code $status_run)."
                falharam=$((falharam + 1))
            fi
        else
            echo "  [FAIL-GCC] $nome_arquivo -> Codigo C gerado apresentou erro sintatico no GCC."
            falharam=$((falharam + 1))
        fi
    else
        # Verifica se é uma limitação esperada de teste negativo
        if [[ "$nome_arquivo" == *"limite"* ]] || [[ "$nome_arquivo" == *"import"* ]]; then
            echo "  [EXPECTED-LIMIT] $nome_arquivo -> Rejeitado pelo compilador (Limitação de Escopo esperada)."
            passaram=$((passaram + 1))
        else
            echo "  [FAIL-COMP] $nome_arquivo -> Erro durante a compilacao do Java."
            falharam=$((falharam + 1))
        fi
    fi
done

rm -rf "$TEMP_DIR"

echo "================================================================="
echo "Resumo dos Testes do Compilador: $passaram passou (incluindo limitações esperadas), $falharam falhou (Total: $total)."
echo "================================================================="

if [ $falharam -gt 0 ]; then
    exit 1
fi
exit 0
