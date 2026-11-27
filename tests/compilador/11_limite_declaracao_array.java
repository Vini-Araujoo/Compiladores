/*
 * TESTE 11: Limitação da Gramática - Declaração de Vetores/Arrays
 * Objetivo: Testar a sintaxe padrão de declaração de array 'int[] v = new int[5];'
 * Diagnóstico esperado: ERRO SINTÁTICO (syntax error).
 *                       A gramática suporta colchetes em parâmetros e expressões new,
 *                       mas não incluiu colchetes na regra de declaração de variáveis locais.
 */
public class LimiteDeclaracaoArray {
    public static void main(String[] args) {
        int[] vetor = new int[5];
        vetor[0] = 42;
        System.out.println(vetor[0]);
    }
}
