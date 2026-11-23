#include "codegen.h"

/*
 * Stubs fracos para controle de fluxo e repeticao (Modulo de Pedro Moretti).
 * Quando a implementacao definitiva for adicionada, estas funcoes serao sobrescritas.
 */

static void emitirIfInterno(NoAST *no, FILE *saida, int nivel,
                            bool isInline){
    if (!no || !saida) return;

    if(isInline){
        fprintf(saida, " ");
    } else {
        emitirIndentacao(saida, nivel);
    }

    fprintf(saida, "if (");
    emitirExpressao(no->filhos[0], saida);
    fprintf(saida, ") {\n");

    if(no->filhos[1]->tipo_no == NO_BLOCO){
        NoAST *no_comando_if = no->filhos[1];
        for(int x = 0; x < no_comando_if->num_filhos; x++){
            emitirInstrucao(no_comando_if->filhos[x], saida, nivel+1);
        }
    } else {
        emitirInstrucao(no->filhos[1], saida, nivel + 1);
    }
    emitirIndentacao(saida, nivel);
    fprintf(saida, "}");
    if(no->num_filhos > 2 && no->filhos[2]){
        NoAST *no_comando_else = no->filhos[2];
        fprintf(saida, " else");
        if(no_comando_else->tipo_no == NO_IF){
            emitirIfInterno(no_comando_else, saida, nivel, true);
        } else {
            fprintf(saida, " {\n");
            if(no_comando_else->tipo_no == NO_BLOCO){
                for(int x = 0; x < no_comando_else->num_filhos; x++){
                    emitirInstrucao(no_comando_else->filhos[x], saida, nivel+1);
                }
            } else {
                emitirInstrucao(no_comando_else, saida, nivel + 1);
            }
            emitirIndentacao(saida, nivel);
            fprintf(saida, "}");
            fprintf(saida, "\n");
        }
    } else {
        fprintf(saida, "\n");
    }
}

void emitirIf(NoAST *no, FILE *saida, int nivel)
{
    if (!no || !saida) return;

    emitirIfInterno(no, saida, nivel, false);
}

void emitirWhile(NoAST *no, FILE *saida, int nivel)
{
    if (!no || !saida) return;

    emitirIndentacao(saida, nivel);
    fprintf(saida, "while (");
    emitirExpressao(no->filhos[0], saida);
    fprintf(saida, ") {\n");
    NoAST *comando_while = no->filhos[1];
    for(int x = 0; x < comando_while->num_filhos; x++){
        emitirInstrucao(comando_while->filhos[x], saida, nivel+1);
    }
    emitirIndentacao(saida, nivel);
    fprintf(saida, "}\n");
}

void emitirDoWhile(NoAST *no, FILE *saida, int nivel)
{
    if (!no || !saida) return;

    emitirIndentacao(saida, nivel);
    fprintf(saida, "do {\n");
    NoAST *comando_do = no->filhos[0];
    for(int x = 0; x < comando_do->num_filhos; x++){
        emitirInstrucao(comando_do->filhos[x], saida, nivel+1);
    }
    emitirIndentacao(saida, nivel);
    fprintf(saida, "} while (");
    emitirExpressao(no->filhos[1], saida);
    fprintf(saida, ");\n");
}

static void emitirAtribuicaoForInline(NoAST *no, FILE *saida)
{
    if (!no) return;

    if (no->tipo_no == NO_DECL_VARIAVEL){
        const char *tipo_c = traduzirTipoC(no->tipo_dado);
        if (!tipo_c || !no->valor.valor_str) return;

        if (no->mods.eh_final) fprintf(saida, "const ");
        size_t tamanho_tipo = strlen(tipo_c);
        if (tamanho_tipo > 0 && tipo_c[tamanho_tipo - 1] == '*'){
            fprintf(saida, "%s%s", tipo_c, no->valor.valor_str);
        } else {
            fprintf(saida, "%s %s", tipo_c, no->valor.valor_str);
        }
        if (no->num_filhos > 0 && no->filhos[0]){
            fprintf(saida, " = ");
            emitirExpressao(no->filhos[0], saida);
        }
        return;
    }

    if (no->tipo_no == NO_ATRIBUICAO){
        if (no->num_filhos < 2 || no->filhos[0] == NULL || no->filhos[1] == NULL) return;

        emitirExpressao(no->filhos[0], saida);
        fprintf(saida, " %s ", no->valor.operador[0] ? no->valor.operador : "=");
        emitirExpressao(no->filhos[1], saida);
        return;
    }
    emitirExpressao(no, saida);
}

void emitirFor(NoAST *no, FILE *saida, int nivel)
{
    if (!no || !saida) return;

    emitirIndentacao(saida, nivel);
    fprintf(saida, "for (");

    NoAST *atribuicao = no->num_filhos > 0 ? no->filhos[0] : NULL;
    if (atribuicao) emitirAtribuicaoForInline(atribuicao, saida);
    fprintf(saida, "; ");

    NoAST *condicao = no->num_filhos > 1 ? no->filhos[1] : NULL;
    if (condicao) emitirExpressao(condicao, saida);
    fprintf(saida, "; ");

    NoAST *incremento = no->num_filhos > 2 ? no->filhos[2] : NULL;
    if (incremento) emitirAtribuicaoForInline(incremento, saida);

    fprintf(saida, ") {\n");
    NoAST *corpo = no->num_filhos > 3 ? no->filhos[3] : NULL;
    if (corpo && corpo->tipo_no == NO_BLOCO){
        for (int i = 0; i < corpo->num_filhos; i++){
            if (corpo->filhos[i]) emitirInstrucao(corpo->filhos[i], saida, nivel + 1);
        }
    }
    emitirIndentacao(saida, nivel);
    fprintf(saida, "}\n");
}

static void emitirCase(NoAST *no_case, FILE *saida, int nivel, bool *hasBreak){
    if(!no_case) return;

    emitirIndentacao(saida, nivel);
    fprintf(saida, "case ");
    emitirExpressao(no_case->filhos[0], saida);
    fprintf(saida, ":\n");
    NoAST *no_bloco = no_case->filhos[1];
    for(int x = 0; x < no_bloco->num_filhos; x++){
        if(no_bloco->filhos[x]->tipo_no == NO_BREAK) *hasBreak = true; 
        emitirInstrucao(no_bloco->filhos[x], saida, nivel+1);
    }
}

void emitirSwitch(NoAST *no, FILE *saida, int nivel)
{
    if (!no || !saida) return;

    emitirIndentacao(saida, nivel);
    fprintf(saida, "switch (");
    emitirExpressao(no->filhos[0], saida);
    fprintf(saida, ") {\n");

    if(no->num_filhos > 1 && no->filhos[1]){
        NoAST *no_cases = no->filhos[1];
        for(int x = 0; x < no_cases->num_filhos; x++){
            bool hasBreak = false;
            emitirCase(no_cases->filhos[x], saida, nivel+1, &hasBreak);
            if(!hasBreak){
                emitirIndentacao(saida, nivel+2);
                fprintf(saida, "break;\n");
            }
        }
    }

    if(no->num_filhos > 2 && no->filhos[2]){
        NoAST *no_default = no->filhos[2];
        emitirIndentacao(saida, nivel+1);
        fprintf(saida, "default:\n");
        bool hasBreak = false;
        NoAST *no_bloco = no_default->filhos[0];
        for(int x = 0; x < no_bloco->num_filhos; x++){
            if(no_bloco->filhos[x]->tipo_no == NO_BREAK) hasBreak = true; 
            emitirInstrucao(no_bloco->filhos[x], saida, nivel+2);
        }
        emitirIndentacao(saida, nivel+2);
        if(!hasBreak) fprintf(saida, "break;\n");
    }
    emitirIndentacao(saida, nivel);
    fprintf(saida, "}\n");
}

__attribute__((weak)) void emitirComandoFluxo(NoAST *no, FILE *saida, int nivel)
{
    (void)no;
    (void)saida;
    (void)nivel;
}
