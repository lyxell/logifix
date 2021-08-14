import java.util.Map;

public class TestRaw {
    public void test(Map map, Object match) {
        /**
         * We don't want to convert this to entrySet since
         * this would require us to cast the result of
         * map.entrySet() to Set<Map.Entry>
         */
        for (Object o : map.keySet()) {
            if (map.get(o) == match) {
                System.out.println("match");
            }
        }
    }
}
