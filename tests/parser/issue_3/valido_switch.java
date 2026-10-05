class ValidoSwitch {
    void executar() {
        switch (opcao) {
            case 1:
                contador = 10;
                break;
            case 2:
                contador = 20;
                break;
            default:
                return 0;
        }
    }
}