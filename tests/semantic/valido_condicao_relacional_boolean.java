class ValidoCondicaoRelacionalBoolean {
    void main() {
        int pontos = 10;
        int minimo = 7;
        boolean aprovado = true;

        if (aprovado && pontos >= minimo) {
        }
        while (pontos > minimo) {
            pontos--;
        }
        do {
            pontos++;
        } while (pontos < minimo);
        for (int i = 0; i < minimo; i++) {
        }
    }
}
