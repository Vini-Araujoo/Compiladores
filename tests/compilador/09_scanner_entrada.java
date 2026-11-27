/*
 * TESTE 09: Entrada de Dados com Scanner
 * Objetivo: Testar instanciação de Scanner e leitura de inteiro via sc.nextInt().
 * Observação/Diagnóstico: No nosso rastreio anterior, vimos que o codegen de Scanner
 *           teve divergência de interface entre decl e io. Teste para ver se compila
 *           ou emite o aviso de suporte.
 */
public class ScannerEntrada {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int numero = 0;
        numero = sc.nextInt();
        System.out.println(numero);
    }
}
