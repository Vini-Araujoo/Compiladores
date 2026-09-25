class ValidoWhileEDoWhile {
    void executar() {
        while (condicao) {
            contador++;
            if (limite) {
                break;
            }
        }

        do {
            contador--;
        } while (condicao);
    }
}