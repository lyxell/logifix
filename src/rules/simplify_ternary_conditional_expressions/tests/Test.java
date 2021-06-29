import java.util.Random;

class Test1 {
    class Inner {
        boolean run() {
            return new Random().nextBoolean();
        }
    }
    boolean test(Inner obj) {
        return obj == null ? false : obj.run();
    }
}
