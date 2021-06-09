import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

class Test {
    void test() {
        try {
            FileInputStream is = new FileInputStream(new File("random/file/path")); // Noncompliant
            byte[] bytes = is.readAllBytes();
            is.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
