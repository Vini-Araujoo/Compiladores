/*
 * TESTE 07: Variáveis Globais de Classe (Dois passos no Codegen)
 * Objetivo: Validar que variáveis declaradas na classe são emitidas no topo do C
 *           antes de todos os métodos, e acessíveis por funções e main.
 * Expectativa: Deve compilar e persistir o estado global.
 */
public class AtributosGlobais {

    private static int contador;
    private static String prefixo;

    public static void incrementar() {
        contador = contador + 1;
    }

    public static void main(String[] args) {
        contador = 10;
        prefixo = "Resultado";

        incrementar();
        incrementar();

        System.out.println(prefixo);
        System.out.println(contador);
    }
}
