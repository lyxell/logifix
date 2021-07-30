import java.util.Random;

class Test1 {
    class Inner {
        boolean run() {
            return new Random().nextBoolean();
        }
    }
    void test1(boolean x) {
        if (x == false) System.out.println("false");
        if (x == true) System.out.println("true");
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
    void test5() {
        Object obj = (Object) new Inner();
        if (obj instanceof Object == false) System.out.println("false");
        if ((obj instanceof Object) == false) System.out.println("false");
        if (((obj instanceof Object)) == false) System.out.println("false");
        if (obj instanceof Object == true) System.out.println("true");
        if ((obj instanceof Object) == true) System.out.println("true");
        if (((obj instanceof Object)) == true) System.out.println("true");
    }
}
