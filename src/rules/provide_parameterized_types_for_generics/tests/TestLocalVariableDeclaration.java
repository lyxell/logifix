import java.util.Deque;
import java.util.ArrayDeque;

public class TestLocalVariableDeclaration {
    public static void main(String[] args) {
        Deque<String> deque = new ArrayDeque();
        System.out.println(deque.size());
    }
}
