import java.util.Collection;
import java.util.ArrayList;

class Test {
    class Inner {
        int length() {
            return 0;
        }
        int size() {
            return 0;
        }
    }
    void test1() {
        String x = "hello world";
        if (x.length() == 0) System.out.println("hello");
        if (x.length() > 0) System.out.println("hello");
        if (x.length() >= 1) System.out.println("hello");
        Inner z = null;
        if (z.length() == 0) System.out.println("hello");
        if (z.length() > 0) System.out.println("hello");
        if (z.length() >= 1) System.out.println("hello");
        if (z.size() == 0) System.out.println("hello");
        if (z.size() > 0) System.out.println("hello");
        if (z.size() >= 1) System.out.println("hello");
    }
    void test2(Collection<Integer> col) {
        if (col.size() == 0) System.out.println("hello");
        if (col.size() > 0) System.out.println("hello");
        if (col.size() >= 1) System.out.println("hello");
    }
    void test3(ArrayList<Integer> list) {
        if (list.size() == 0) System.out.println("hello");
        if (list.size() > 0) System.out.println("hello");
        if (list.size() >= 1) System.out.println("hello");
    }
}
