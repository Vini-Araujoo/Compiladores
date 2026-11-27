/*
 * TESTE 01: Aritmética, Precedência e Tipos Primitivos
 * Objetivo: Testar cálculos matemáticos, precedência de operadores (+, -, *, /, %)
 *           e declaração de múltiplos tipos primitivos com e sem inicialização.
 * Expectativa: Deve compilar e executar com 100% de sucesso.
 */
public class AritmeticaETipos {
    public static void main(String[] args) {
        int a = 10;
        int b = 20;
        int c = a + b * 2 - (15 / 3) + (8 % 3);

        double pi = 3.14;
        double raio = 2.0;
        double area = pi * raio * raio;

        boolean ativo = true;
        char letra = 'X';
        String saudacao = "Compiladores 2026";

        System.out.println(c);
        System.out.println(area);
        System.out.println(ativo);
        System.out.println(letra);
        System.out.println(saudacao);
    }
}
