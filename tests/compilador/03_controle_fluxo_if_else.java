/*
 * TESTE 03: Controle de Fluxo (if / else if / else) e Operadores Lógicos
 * Objetivo: Testar as regras novas de fluxo de Pedro Moretti com
 *           condições encadeadas e operadores lógicos (&&, ||, !).
 * Expectativa: Deve compilar e executar imprimindo o fluxo correto.
 */
public class ControleFluxoIfElse {
    public static void main(String[] args) {
        int x = 15;
        int y = 20;

        if (x > 10 && y < 30) {
            System.out.println(1);
        } else {
            System.out.println(0);
        }

        if (x == 100 || !(y > 50)) {
            System.out.println(2);
        }

        if (x < 5) {
            System.out.println(10);
        } else if (x == 15) {
            System.out.println(20);
        } else {
            System.out.println(30);
        }
    }
}
