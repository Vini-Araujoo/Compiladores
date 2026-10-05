class ValidoAninhamentoERetorno {
    void executar() {
        {
            int x = 10;
            if (x) {
                while (x) {
                    return x;
                }
            }
            return;
        }
    }
}