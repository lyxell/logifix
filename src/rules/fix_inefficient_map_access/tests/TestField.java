import java.util.Map;
import java.util.HashMap;

public class TestField {

    public Map<Integer, String> map;

    public HashMap<Integer, String> hashMap;

    public void run1() {
        for (Integer key : map.keySet()) {
            System.out.println(key);
            System.out.println(map.get(key));
        }
    }

    public void run2() {
        for (Integer key : hashMap.keySet()) {
            System.out.println(key);
            System.out.println(hashMap.get(key));
        }
    }

}
