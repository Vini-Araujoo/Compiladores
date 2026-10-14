class ValidoIfCondicaoComposta {
    void executar() {
        if (ativo && (pontuacao >= 10 || aprovado)) {
            resultado = 1;
        } else {
            resultado = 0;
        }
    }
}
