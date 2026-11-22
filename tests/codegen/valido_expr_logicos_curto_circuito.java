class ExprLogicos {
    void executar() {
        int a = 3;
        int b = 0;
        boolean flag = true;
        boolean r1 = a > 0 && b < 10;
        boolean r2 = a > 5 || b >= 0;
        boolean r3 = !flag;
        boolean r4 = !(a > 0 && b < 10);
        boolean r5 = flag || a < 0 && b < 0;
        boolean r6 = (flag || a < 0) && b < 0;
        boolean r7 = b > 0 && a / b > 1;
        boolean r8 = b <= 0 || a / b > 1;
        boolean r9 = !(a < 0) && !flag || b < 0;
    }
}