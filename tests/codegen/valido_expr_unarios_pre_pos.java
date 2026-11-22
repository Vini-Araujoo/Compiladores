class ExprUnarios {
    void executar() {
        int a = 5;
        int b = 10;
        int c = -a;
        int d = -(a + b);
        int e = -(-a);
        int f = a++;
        int g = ++a;
        int h = b--;
        int i = --b;
        int j = a++ + ++b;
        int k = -a++;
        a++;
        --b;
        int l = a-- - --b;
        boolean m = !(a++ > 7);
        double n = -2.5;
    }
}