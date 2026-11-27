/*
 * TESTE 04: Laços de Repetição (while, do-while e for)
 * Objetivo: Testar os 3 tipos de laços iterativos com incremento e decremento.
 * Expectativa: Deve compilar e executar imprimindo as sequências.
 */
public class LacosRepeticao {
    public static void main(String[] args) {
        // 1. Laço While
        int i = 0;
        while (i < 3) {
            System.out.println(i);
            i++;
        }

        // 2. Laço Do-While
        int j = 10;
        do {
            System.out.println(j);
            j--;
        } while (j > 8);

        // 3. Laço For
        for (int k = 1; k <= 3; k++) {
            System.out.println(k);
        }
    }
}
