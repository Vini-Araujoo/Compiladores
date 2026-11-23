class ValidoFluxoWhileDoWhile {
    void main() {
        int contador = 0;
        int total = 0;

        while (contador < 3) {
            total += contador;
            contador++;
        }

        int repeticoes = 0;
        do {
            repeticoes++;
        } while (repeticoes < 2);

        System.out.println(total);
        System.out.println(repeticoes);
    }
}
