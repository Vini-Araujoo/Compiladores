
public class Main {

    public static void main(String[] args) {

        int l;

        int val = 0;

        Scanner n = new Scanner(System.in);

        val = n.nextInt();

        if (val < 1) {
            System.out.println(val);
            return;
        }

        for (int i = 0; i < val;) {
            i++;
            System.out.println(i);
        }

    }

}
