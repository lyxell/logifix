import java.util.Random;

class Test1 {
    boolean getBoolean() {
        return new Random().nextBoolean();
    }
    boolean test1(boolean x) {
        return x ? true : getBoolean();
    }
    boolean test2(boolean x) {
        return x ? false : getBoolean();
    }
    boolean test3(boolean x) {
        return x ? getBoolean() : true;
    }
    boolean test4(boolean x) {
        return x ? getBoolean() : false;
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
    Boolean test10(Object x, boolean y) {
        return x == null ? true : null;
    }
}
