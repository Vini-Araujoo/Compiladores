/*
 * TESTE 05: Estrutura Switch-Case com Break e Default
 * Objetivo: Validar o switch-case em inteiros, saltos de break e fallback do default.
 * Expectativa: Deve compilar e imprimir os casos correspondentes.
 */
public class SwitchCase {
    public static void main(String[] args) {
        int opcao = 2;

        switch (opcao) {
            case 1:
                System.out.println(100);
                break;
            case 2:
                System.out.println(200);
                break;
            case 3:
                System.out.println(300);
                break;
            default:
                System.out.println(999);
                break;
        }
    }
}
