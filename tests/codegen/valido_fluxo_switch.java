class ValidoFluxoSwitch {
    void main() {
        int valor = 1;
        int resultado = 0;

        // case escolhido com break explicito
        switch (valor) {
            case 1:
                resultado = 10;
                break;
            case 2:
                resultado = 20;
            default:
                resultado = 30;
                break;
        }
        System.out.println(resultado);

        // case sem break explicito: o gerador deve encerrar o case
        valor = 2;
        switch (valor) {
            case 1:
                resultado = 11;
            case 2:
                resultado = 21;
            default:
                resultado = 31;
        }
        System.out.println(resultado);

        // caminho default
        valor = 9;
        switch (valor) {
            case 1:
                resultado = 12;
            default:
                resultado = 32;
        }
        System.out.println(resultado);

        // switch sem default e sem case correspondente
        switch (valor) {
            case 0:
                resultado = 99;
                break;
        }
        System.out.println(resultado);
    }
}
