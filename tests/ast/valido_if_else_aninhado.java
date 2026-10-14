class ValidoIfElseAninhado {
    void executar() {
        if (condicaoExterna)
            if (condicaoInterna)
                resultado = 1;
            else
                resultado = 2;
        else
            resultado = 3;
    }
}
