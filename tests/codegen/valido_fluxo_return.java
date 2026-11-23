class ValidoFluxoReturn {
    int dobrar(int valor) {
        if (valor > 0) {
            return valor * 2;
        }
        return 0;
    }

    void retornarVazio() {
        return;
    }

    void main() {
        retornarVazio();
        int resultado = dobrar(4);
        System.out.println(resultado);
    }
}
