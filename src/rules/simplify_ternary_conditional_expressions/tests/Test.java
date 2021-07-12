import java.util.Random;

class Test1 {
    class Inner {
        boolean run() {
            return new Random().nextBoolean();
        }
    }
    boolean test1(boolean x, Inner obj) {
        return x ? true : obj.run();
    }
    boolean test2(boolean x, Inner obj) {
        return x ? false : obj.run();
    }
    boolean test3(boolean x, Inner obj) {
        return x ? obj.run() : true;
    }
    boolean test4(boolean x, Inner obj) {
        return x ? obj.run() : false;
    }
    boolean test5(boolean x) {
        return x ? true : false;
    }
    boolean test6(boolean x) {
        return x ? false : true;
    }
    boolean test7(boolean x) {
        return x   ?   (false) :  ( (  true) );
    }
    boolean test8(Object x, boolean y) {
        return x == null  ?   (y) :  ( (  true) );
    }
    boolean test9(Object x, boolean y) {
        return ((x == null))  ?   (y) :  ( (  true) );
    }
}
