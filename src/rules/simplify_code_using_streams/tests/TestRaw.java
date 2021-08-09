import java.util.ArrayList;

public class Test {
    public static void main(String[] args) {
        Object z = new Integer(0);
        ArrayList l1 = new ArrayList();
        l1.add(0);
        ArrayList l2 = new ArrayList();
        for (Object o : l1) {
            if (o.equals(z)) {
                l2.add(o);
            }
        }
        System.out.println(l2.size());
    }
}
