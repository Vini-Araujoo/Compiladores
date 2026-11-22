class ExprRelacionais {
    void executar() {
        int a = 5;
        int b = 10;
        double x = 2.5;
        boolean igual = a == b;
        boolean diferente = a != b;
        boolean menor = a < b;
        boolean menorIgual = a <= 5;
        boolean maior = b > a;
        boolean maiorIgual = a >= b;
        boolean mista = x > a;
        boolean composta = a + 1 < b * 2;
        boolean comParenteses = (a + b) > (b - a) * 2;
    }
}