class ExprComposta {
    void executar() {
        int a = 10;
        int b = 4;
        int c = 6;
        int d = 2;
        int r1 = (a + b) * (c - d);
        int r2 = a - (b - c);
        int r3 = a - b - c;
        int r4 = a / (b * d);
        int r5 = a % b * c;
        int r6 = a * (b % c);
        int r7 = a + b * c - d / 2;
    }
}