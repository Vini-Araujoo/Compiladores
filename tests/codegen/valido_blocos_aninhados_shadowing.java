class TesteBlocosAninhadosShadowing {
    public static void main(String[] args) {
        int x = 10;
        {
            int x = 20;
            x += 5;
            {
                int x = 30;
                int y = x + 1;
                y -= 2;
            }
            x -= 3;
        }
        x = x + 1;
    }
}
