class ValidoFluxoFor {
    void main() {
        int total = 0;

        for (int i = 0; i < 4; i++) {
            total += i;
        }

        int j = 0;
        for (j = 0; j < 2; j += 1) {
            total += j;
        }

        System.out.println(total);
    }
}
