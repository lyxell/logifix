public class TestMethodInvocation {
    boolean isEmpty(String s) {
        return s == null || s.isEmpty();
    }
    void test(String test) {
        // redundant parentheses around isEmpty
        if (!(isEmpty(test))) {
            System.out.println("is empty");
        }
    }
}
