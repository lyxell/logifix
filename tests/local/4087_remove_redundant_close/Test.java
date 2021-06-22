import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

class Test {
    void test() {
        try (FileInputStream is = new FileInputStream(new File("Test.java"))) {
            byte[] bytes = is.readAllBytes();
            is.close();
            System.out.println(Arrays.toString(bytes));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
