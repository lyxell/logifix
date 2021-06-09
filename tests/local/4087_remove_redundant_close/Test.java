import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

class Test {
    void test() {
        try (FileInputStream is = new FileInputStream(new File("foo"))) {
            byte[] bytes = is.readAllBytes();
            is.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
