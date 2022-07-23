public class Test {
    public void test(boolean x, boolean y, boolean z, boolean w) {
        if (x || y) {
            // empty
        }

        if (x && y) {
            System.out.println("hello");
        } else if (z) {
            // empty
        }

        if (x || y) {
            // empty
        } else if (z) {
            // empty
        } else if (w) {
            System.out.println("hello");
        }

    }
}
