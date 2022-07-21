import java.util.Map;
import java.util.List;

public class TestFalsePositive {

    public void test(List<String> strings, Map<String, List<String>> map) {
        strings.forEach((str) -> map.computeIfAbsent(str, (k) -> new ArrayList<>()).add(str));
    }
}
