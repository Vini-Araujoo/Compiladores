/*
 * TESTE 13: Limitação da Gramática - Cláusula Import
 * Objetivo: Testar o uso comum de 'import java.util.Scanner;' no topo do arquivo.
 * Diagnóstico esperado: ERRO SINTÁTICO na linha 1.
 *                       O compilador trata Scanner e System como primitivas nativas
 *                       e não reconhece a palavra-chave 'import'.
 */
import java.util.Scanner;

public class LimiteImport {
    public static void main(String[] args) {
        System.out.println(1);
    }
}
