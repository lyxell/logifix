import java.util.List;

public class TestTypeCheck {
    public static void main(String[] args) {
        List<String> list = List.of("Hello", "Hi");
        list.stream().map(x -> x.length()).forEach(x -> System.out.println(x));
    }
}
