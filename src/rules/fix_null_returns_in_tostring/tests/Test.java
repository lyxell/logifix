import java.util.Random;

public class Test {
    public String toString() {
        Random r = new Random();
        if (r.nextBoolean()) {
            return null;
        }
        return "";
    }
}
