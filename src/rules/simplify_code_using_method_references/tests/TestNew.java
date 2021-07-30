import java.util.List;
import java.util.Arrays;
import java.util.stream.Collectors;

public class TestNew {
    static class Inner {
        private int y = 5;
        public Inner(int y) {
            this.y = y;
        }
        public String toString() {
            return "this is " + y; 
        }
    }
	public static void main(String[] args) {
        List<Integer> x = Arrays.asList(1, 2, 3, 4);
        List<Inner> result = x.stream().map(n -> new Inner(n)).collect(Collectors.toList());
        System.out.println(result);
	}
}
