class ValidoReturnFormas {
    void executar() {
        if (encerrar) {
            return;
        }

        if (sucesso) {
            return resultado;
        }

        return 0;
    }
}