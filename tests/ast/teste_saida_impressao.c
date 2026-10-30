#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "../../src/ast.h"

int main(void) {
    printf("Executando teste_saida_impressao...\n");

    /* 1. Montar uma arvore de teste para impressao */
    NoAST *classe = criarNo(NO_CLASSE, 1);
    classe->valor.valor_str = strdup("Exemplo");

    NoAST *metodo = criarNo(NO_METODO, 2);
    metodo->valor.valor_str = strdup("executar");
    adicionarFilho(classe, metodo);

    NoAST *params = criarNo(NO_PARAMETROS, 2);
    adicionarFilho(metodo, params);

    NoAST *p1 = criarNo(NO_PARAMETRO, 2);
    p1->valor.valor_str = strdup("x");
    adicionarFilho(params, p1);

    NoAST *bloco = criarNo(NO_BLOCO, 3);
    adicionarFilho(metodo, bloco);

    /* 2. Redirecionar stdout para arquivo temporario */
    fflush(stdout);
    int stdout_backup = dup(STDOUT_FILENO);
    FILE *tmp = tmpfile();
    assert(tmp != NULL);
    dup2(fileno(tmp), STDOUT_FILENO);

    /* 3. Imprimir a arvore */
    imprimirAST(classe);
    fflush(stdout);

    /* 4. Restaurar stdout */
    dup2(stdout_backup, STDOUT_FILENO);
    close(stdout_backup);

    /* 5. Ler saida e comparar com termos esperados */
    rewind(tmp);
    char buffer[2048] = {0};
    size_t lidos = fread(buffer, 1, sizeof(buffer) - 1, tmp);
    buffer[lidos] = '\0';
    fclose(tmp);

    /* 6. Validar que contem os nos e a estrutura esperada */
    assert(strstr(buffer, "NO_CLASSE (Exemplo)") != NULL);
    assert(strstr(buffer, "NO_METODO (executar)") != NULL);
    assert(strstr(buffer, "NO_PARAMETROS") != NULL);
    assert(strstr(buffer, "NO_PARAMETRO (x)") != NULL);
    assert(strstr(buffer, "NO_BLOCO") != NULL);

    /* 7. Liberar arvore */
    liberarAST(classe);

    printf("teste_saida_impressao passou com sucesso!\n");
    return 0;
}
