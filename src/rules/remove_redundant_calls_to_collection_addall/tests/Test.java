import java.util.List;
import java.util.ArrayList;

class Test {
    void test() {
        List<Integer> a = List.of(1, 2, 3);
        List<Integer> b = new ArrayList<>();
        b.addAll(a);
        a.add(4);
        /* Should print 1, 2, 3 */
        System.out.println(b);
    }
}
