import java.util.ArrayList;
import java.util.List;

public class Test {
    public static void main(String[] args) {
        List<Integer> list = new ArrayList<>();
        list.add(3);
        list.add(9);
        System.out.println(list);
        list.removeAll(list);
        System.out.println(list);
    }
}
