import java.util.Map;

public class Test {
    public static void run(Map<Integer,String> map) {
        for (Integer key : map.keySet()) {
            System.out.println(key);
            System.out.println(map.get(key));
        }
    }
}
