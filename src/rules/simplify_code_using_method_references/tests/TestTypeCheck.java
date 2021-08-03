import java.util.List;
import java.util.stream.Collectors;

public class TestTypeCheck {
    public static void main(String[] args) {
        List<String> list = List.of("Hello", "Hi");
        // map
        list.stream().map(x -> x.length()).forEach(x -> System.out.println(x));
        // filter
        list.stream().filter(x -> x.isEmpty()).forEach(x -> System.out.println(x));
    }
    public void test1(List<String> list) {
        // map
        list.stream().map(x -> x.length()).forEach(x -> System.out.println(x));
        // filter
        list.stream().filter(x -> x.isEmpty()).forEach(x -> System.out.println(x));
    }
    public void test2(List<String> list1, List<String> list2) {
        list2.addAll(list1.stream().filter(x -> x.isEmpty()).collect(Collectors.toList()));
    }
}
