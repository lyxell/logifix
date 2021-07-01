import java.util.ArrayList;

public class TestMethodReference {
    public void print(Integer i) {
        System.out.println(i);
    }
    public static void main(String[] args) {
        ArrayList<Integer> list = new ArrayList<>();
        list.add(1);
        list.add(2);
        TestMethodReference t1 = new TestMethodReference();
        TestMethodReference t2 = new TestMethodReference();
        list.forEach(t1::print);
    }
}
