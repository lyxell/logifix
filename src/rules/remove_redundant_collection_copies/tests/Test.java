import java.util.List;
import java.util.ArrayList;
import java.util.stream.Collectors;

class Test {
    List<Integer> test(List<Integer> original) {
        return new ArrayList<>(original.stream().filter(x > 0).collect(Collectors.toList()));
    }
}