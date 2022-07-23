import java.util.Set;
import java.util.Iterator;

class TestIterator {
    public void test(Set<Integer> set) {
        Iterator<Integer> it = set.iterator();
        for (; it.hasNext();) {
            System.out.println((Integer) it.next());
        }
        for (Iterator<Integer> x = set.iterator(); x.hasNext();) {
            System.out.println((Integer) x.next());
        }
    }
}
