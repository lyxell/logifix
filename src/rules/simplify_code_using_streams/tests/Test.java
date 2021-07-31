import java.util.ArrayList;
import java.util.List;

public class Test {
    public static void main(String[] args) {
        ArrayList<Integer> original = new ArrayList<>();
        original.add(1);
        original.add(2);
        original.add(3);
        original.add(4);
        List<Integer> result = new ArrayList<>();
        for (Integer x : original) {
            if (x > 1) {
                result.add(x);
            }
        }
        System.out.println(result);
    }
}
