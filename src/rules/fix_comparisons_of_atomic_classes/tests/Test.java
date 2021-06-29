import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

class Test {
    void test1() {
        AtomicBoolean a = new AtomicBoolean(true);
        AtomicBoolean b = new AtomicBoolean(false);
        System.out.println(a.equals(b));
    }
    void test2() {
        AtomicInteger a = new AtomicInteger(0);
        AtomicInteger b = new AtomicInteger(1);
        System.out.println(a.equals(b));
    }
    void test3() {
        AtomicLong a = new AtomicLong(0);
        AtomicLong b = new AtomicLong(1);
        System.out.println(a.equals(b));
    }
}
