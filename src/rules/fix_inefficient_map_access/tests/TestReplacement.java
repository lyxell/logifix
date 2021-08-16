import java.util.Map;

public class TestReplacement {
    public void test1(Map<Integer,String> map) {
        for (Map.Entry<Integer, String> entry : map.entrySet()) {
            System.out.println(map.get(entry.getKey()));
        }
    }
    public void test2(Map<Integer,String> map) {
        for (Map.Entry<Integer, String> entry : map.entrySet()) {
            Integer key = entry.getKey();
            System.out.println(map.get(key));
        }
    }
}
