public class TestExpressionName {
    void test(boolean isEmpty) {
        // redundant parentheses around isEmpty
        if (!(isEmpty)) {
            System.out.println("is empty");
        }
    }
}
