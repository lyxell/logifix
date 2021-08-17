import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.stream.Collectors;

class TestFalsePositives {
    ArrayList<Integer> test(List<Integer> original) {
        return new ArrayList<>(original.stream().filter(x -> x > 0).collect(Collectors.toList()));
    }
    ArrayList<Integer> testAsList(List<Integer> original) {
        return new ArrayList<>(Arrays.asList(1,2,3));
    }
    void testAssignment(List<Integer> original) {
        ArrayList<Integer> list = new ArrayList<>(original.stream().filter(x -> x > 0).collect(Collectors.toList()));
        System.out.println(list.size());
    }
    void testAsListAssignment(List<Integer> original) {
        ArrayList<Integer> list = new ArrayList<>(Arrays.asList(1,2,3));
        System.out.println(list.size());
    }
}
