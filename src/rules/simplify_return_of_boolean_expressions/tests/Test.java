class Test {
    boolean test1(int x) {
        if (x == 20) {
            return true;
        } else {
            return false;
        }
    }
    boolean test2(int x) {
        if (x == 20) return true;
        else return false;
    }
    boolean test3(int x) {
        if (x == 20) {
            return false;
        } else {
            return true;
        }
    }
    boolean test4(int x) {
        if (x == 20) return false;
        else return true;
    }
    boolean test5(int x) {
        if (x == 20) return false;
        else return true;
    }
    boolean test6(int x) {
        if (x == 20) return ( false );
        else return (  true  );
    }

    boolean test7(int x) {
        if    (x == 20)
                return  ( false );
        else
                return   ( ( (true) ) );
    }
}
