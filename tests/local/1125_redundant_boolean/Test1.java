import java.util.Random;

public class Test1 {
    public void test1(boolean x) {
        if (x == false) System.out.println("false");
        if (x == true) System.out.println("true");
    }
    class Inner {
        boolean run() {
            return new Random().nextBoolean();
        }
    }
    public void test2(Inner obj) {
        return obj == null ? false : obj.run();
    }
    public void test3(int x) {
        if ((x == 3) == false) System.out.println("false");
        if ((x == 3 || x == 4) == false) System.out.println("false");
    }
    public void test4() {
        Inner obj = new Inner();
        if (obj.run() == false) System.out.println("false");
        if (obj.run() == true) System.out.println("true");
    }
}
