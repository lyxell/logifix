import java.util.List;

public class Test {
    public static void main(String[] args) {
        List<Integer> l = List.of(1,2,3);
        System.out.println(l.stream().map(x -> x + 1).collect(java.util.stream.Collectors.toList()));
    }
}
