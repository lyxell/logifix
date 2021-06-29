import java.util.Random;

class Test1 {
    void test1(boolean x) {
        if (x == false) System.out.println("false");
        if (x == true) System.out.println("true");
    }
    class Inner {
        boolean run() {
            return new Random().nextBoolean();
        }
    }
    boolean test2(Inner obj) {
        return obj == null ? false : obj.run();
    }
    void test3(int x) {
        if ((x == 3) == false) System.out.println("false");
        if ((x == 3 || x == 4) == false) System.out.println("false");
    }
    void test4() {
        Inner obj = new Inner();
        if (obj.run() == false) System.out.println("false");
        if (obj.run() == true) System.out.println("true");
    }
}
