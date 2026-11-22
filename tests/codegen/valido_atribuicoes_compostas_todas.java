class TesteAtribuicoesCompostas {
    public static void main(String[] args) {
        int total = 100;
        int passo = 10;

        total += 25;
        total -= 15;
        total += passo * 2;
        total -= passo + 5;

        total++;
        ++total;
        total--;
        --total;

        total = total * 2;
        total = total / 4;
        total = total % 7;
    }
}
