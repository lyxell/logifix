import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Arrays;

public class Test {
    public static void main(String[] args) {
        try {
            FileInputStream is = new FileInputStream(new File("Test.java")); // Noncompliant
            byte[] bytes = is.readAllBytes();
            System.out.println(Arrays.toString(bytes));
            is.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
