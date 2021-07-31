import java.util.List;
import java.util.ArrayList;

public class TestMethod {
    public List<Integer> getNumbers() {
        return List.of(-10, -5, 0, 1, 3, 9);
    }
    public List<Integer> getPositiveNumbers() {
        List<Integer> positiveNumbers = new ArrayList<>();
        for (Integer i : getNumbers()) {
            if (i > 0) {
                positiveNumbers.add(i);
            }
        }
        return positiveNumbers;
    }
}
