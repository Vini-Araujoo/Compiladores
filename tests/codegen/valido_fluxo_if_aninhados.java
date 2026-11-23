class ValidoFluxoIfAninhados {
    void main() {
        int x = 3;
        int y = -1;
        int resultado = 0;

        if (x >= 0) {
            if (x > 2) {
                resultado = 10;
            } else {
                resultado = 20;
            }
        }

        if (x < 0) {
            resultado = 30;
        } else if (y < 0) {
            resultado += 5;
        } else {
            resultado = 40;
        }

        System.out.println(resultado);
    }
}
