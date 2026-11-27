/*
 * TESTE 14: Ordem de Definição - Método Chamado Antes de Ser Definido
 * Objetivo: Chamar o método 'somar' na main antes de sua implementação abaixo.
 * Diagnóstico esperado: Em Java é 100% válido. Em C, sem protótipos de função adiantados,
 *                       o GCC pode emitir aviso de 'implicit-function-declaration'.
 */
public class MetodoDeclaradoDepois {
    public static void main(String[] args) {
        int r = multiplicar(4, 5);
        System.out.println(r);
    }

    public static int multiplicar(int a, int b) {
        return a * b;
    }
}
