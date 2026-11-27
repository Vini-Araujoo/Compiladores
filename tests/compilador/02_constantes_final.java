/*
 * TESTE 02: Constantes com modificador final
 * Objetivo: Validar que variáveis 'final' são traduzidas para 'const' em C
 *           e verificar uso de declarações múltiplas na mesma linha.
 * Expectativa: Deve compilar e executar com sucesso.
 */
public class ConstantesFinal {
    public static void main(String[] args) {
        final int MAX = 100;
        final double TAXA = 0.05;
        int valor = 50;

        int total = valor + MAX;
        System.out.println(total);
        System.out.println(TAXA);
    }
}
