#ifndef AST
#define AST

typedef struct noAST{
    TipoNoAST tipo_no;
    TipoDado tipo_dado;
    struct NoAST **filhos;
    int num_filhos;
    int linha;
}NoAST;

#endif // !1