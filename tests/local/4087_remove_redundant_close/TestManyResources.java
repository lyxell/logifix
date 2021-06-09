import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

class Test {
    void test() {
        try (FileInputStream is1 = new FileInputStream(new File("foo"));
                FileInputStream is2 = new FileInputStream(new File("foo"))) {
            byte[] bytes = is1.readAllBytes();
            byte[] bytes = is2.readAllBytes();
            is1.close();
            is1.close();
            is2.close();
            is1.close();
            is1.close();
            is2.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
