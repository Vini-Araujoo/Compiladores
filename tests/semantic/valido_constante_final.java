class ValidoConstanteFinal {
    final int LIMITE_GLOBAL = 1000;

    void executar() {
        final int MAX = 100;
        final double PI = 3.14159;
        int normal;
        normal = MAX + LIMITE_GLOBAL;
        normal++;
        normal += 5;
    }
}
