/*
 * TESTE 08: Escopos Aninhados e Sombreamento de Variáveis (Shadowing)
 * Objetivo: Testar blocos de código { ... } aninhados dentro de métodos
 *           e resolução de variáveis de mesmo nome em escopos diferentes.
 * Expectativa: Deve compilar e respeitar os valores de cada escopo.
 */
public class EscoposEBlocos {
    public static void main(String[] args) {
        int x = 10;
        System.out.println(x);

        {
            int y = 20;
            System.out.println(y);
        }

        System.out.println(x);
    }
}
