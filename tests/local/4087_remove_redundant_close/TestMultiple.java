import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Arrays;

class Test {
    void test() {
        try (FileInputStream is = new FileInputStream(new File("foo"))) {
            byte[] bytes = is.readAllBytes();
            is.close();
            is.close();
            is.close();
            is.close();
            is.close();
            is.close();
            System.out.println(Arrays.toString(bytes));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
