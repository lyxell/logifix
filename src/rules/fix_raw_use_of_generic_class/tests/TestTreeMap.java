import java.util.Map;
import java.util.TreeMap;

public class TestTreeMap {
 	public Map<String, Integer> test() {
		Map<String, Integer> values = new TreeMap();
        values.put("hello", 0);
        return values;
    }
}
