import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Arrays;

public class Test {
    public static void main(String[] args) {
        try (FileInputStream is = new FileInputStream(new File("Test.java"))) {
            byte[] bytes = is.readAllBytes();
            System.out.println(Arrays.toString(bytes));
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // do nothing
        }

        try {
            int x = 62 / 0;
            System.out.println(x);
        } catch (ArithmeticException e) {
            e.printStackTrace();
        } finally {

        }

    }
}
