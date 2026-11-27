/*
 * TESTE 12: Limitação Semântica - Operador de Igualdade em Expressão Booleana
 * Objetivo: Testar a atribuição de uma comparação 'boolean b = (3 == 3);'
 * Diagnóstico esperado: ERRO SEMÂNTICO.
 *                       A função de inferência binária esqueceu os operadores '==' e '!=',
 *                       promovendo-os para int em vez de boolean.
 */
public class LimiteIgualdadeBooleana {
    public static void main(String[] args) {
        boolean resultado = (3 == 3);
        System.out.println(resultado);
    }
}
