import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.stream.Collectors;

class Test {
    List<Integer> test(List<Integer> original) {
        return new ArrayList<>(original.stream().filter(x -> x > 0).collect(Collectors.toList()));
    }
    List<Integer> testAsList(List<Integer> original) {
        return new ArrayList<>(Arrays.asList(1,2,3));
    }
}
