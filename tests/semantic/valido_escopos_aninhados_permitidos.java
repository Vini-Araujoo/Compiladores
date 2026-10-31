class ValidoEscoposAninhadosPermitidos {
    int x;
    int y;

    void executar(int a) {
        int x;
        x = 10;
        y = a + x;
        {
            int x;
            int b;
            x = 20;
            b = x + y;
        }
        {
            int b;
            b = 30;
            x = b;
        }
    }
}
