import java.util.Map;
import java.util.List;
import java.util.ArrayList;

public class TestSimple {
    public test(Map<Integer, List<String>> map, Integer key) {
        if (!map.containsKey(key)) {
            map.put(key, new ArrayList<>());
        }
    }
}
