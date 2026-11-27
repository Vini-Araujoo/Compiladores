/*
 * TESTE 10: Limitação - Concatenação de String com Inteiro via '+'
 * Objetivo: Demonstrar o comportamento de 'String + int' traduzido diretamente para C.
 * Diagnóstico esperado: Em Java daria "Valor: 5".
 *                       Em C, como gera (nome + 5), ocorre ARITMÉTICA DE PONTEIROS
 *                       e a string é impressa deslocada (ou comportamento inesperado).
 */
public class AnomaliaSomaString {
    public static void main(String[] args) {
        String texto = "abcdef";
        int deslocamento = 2;

        // Em C: printf("%s\n", (texto + deslocamento)) -> imprime "cdef"
        System.out.println(texto + deslocamento);
    }
}
