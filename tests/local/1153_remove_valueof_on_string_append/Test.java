class Test {
    void test1() {
        String s = "hello ";
        float val = 2.0f;
        System.out.println("hello " + String.valueOf(val)); // Noncompliant
        System.out.println(s + String.valueOf(val)); // Noncompliant
    }
    void test2() {
        String s = "hello ";
        boolean val = true;
        System.out.println("hello " + String.valueOf(val)); // Noncompliant
        System.out.println(s + String.valueOf(val)); // Noncompliant
    }
    void test3() {
        String s = "hello ";
        int val = 2;
        System.out.println("hello " + String.valueOf(val)); // Noncompliant
        System.out.println(s + String.valueOf(val)); // Noncompliant
    }
}
