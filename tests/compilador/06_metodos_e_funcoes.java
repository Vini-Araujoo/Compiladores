public class MetodosEFuncoes {

    public static int somar(int a, int b) {
        return a + b;
    }

    public static int fatorial(int n) {
        if (n <= 1) {
            return 1;
        }
        int temp = fatorial(n - 1);
        return n * temp;
    }

    public static void main(String[] args) {
        int s = somar(15, 25);
        int fat = fatorial(5);

        System.out.println(s);
        System.out.println(fat);
    }
}
