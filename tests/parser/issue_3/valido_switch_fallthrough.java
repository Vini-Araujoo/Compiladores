class ValidoSwitchFallthrough {
    void executar() {
        switch (codigo) {
            case 1:
                resultado = 10;
            case 2:
                resultado = 20;
                break;
            default:
                resultado = 0;
        }
    }
}