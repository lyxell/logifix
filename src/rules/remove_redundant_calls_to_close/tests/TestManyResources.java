import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Arrays;

class Test {
    void test() {
        try (FileInputStream is1 = new FileInputStream(new File("foo"));
                FileInputStream is2 = new FileInputStream(new File("foo"))) {
            byte[] bytes1 = is1.readAllBytes();
            byte[] bytes2 = is2.readAllBytes();
            is1.close();
            is1.close();
            is2.close();
            is1.close();
            is1.close();
            is2.close();
            System.out.println(Arrays.toString(bytes1));
            System.out.println(Arrays.toString(bytes2));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
