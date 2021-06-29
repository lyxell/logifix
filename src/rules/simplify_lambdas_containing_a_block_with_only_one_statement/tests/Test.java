import java.util.ArrayList;

public class TestLambda {
    public static void main(String[] args) {
        ArrayList<Integer> numbers = new ArrayList<Integer>();
        numbers.add(1);
        numbers.add(2);
        numbers.add(3);
        numbers.add(4);
        numbers.add(5);
        numbers.removeIf(n -> {
            return n % 2 == 0;
        });
        System.out.println(numbers);
    }
}
