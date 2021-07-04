import java.util.Collection;
import java.util.ArrayList;
import java.util.Iterator;

public class TestCollectionIterator {
    public static void main(String[] args) {
        Collection<Integer> coll = new ArrayList<>();
        coll.add(1);
        coll.add(2);
        Iterator it = coll.iterator();
        System.out.println(it.next());
    }
}
