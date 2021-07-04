import java.util.Map;

public class Test {

    Map<String, Integer> myMap;

    Integer test(String key) {
        Integer result = myMap.get(key);
        if (result == null) {
            result = key.length() + 10;
            myMap.put(key, result);
        }
        return result;
    }

    public static void main(String[] args) {
        Test test = new Test();
        System.out.println(test.test("hello"));
    }
}
