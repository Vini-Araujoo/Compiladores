class ValidoFluxoIfElse {
    void main() {
        int x = 3;
        int resultado = 0;

        // if sem else
        if (x < 0) {
            resultado = 100;
        }

        // if-else com expressao aritmetica e atribuicao
        if (x > 0) {
            resultado = x * 2;
        } else {
            resultado = 1;
        }

        System.out.println(resultado);
    }
}
