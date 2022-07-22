import java.util.Optional;

public class TestOptional {
    public static void main(String[] args) {
        String name = "test";
        Optional<String> opt = Optional.of(name);
        System.out.println(opt.map(x -> x.length()).get());
    }
}
