class Test {
    void test() {
        String str = "hello world";
        System.out.println(str.substring(0)); // Noncompliant
        System.out.println(str.substring(0, str.length())); // Noncompliant
        System.out.println(str.substring(str.length())); // Noncompliant
        System.out.println(str.substring(3, str.length())); // Noncompliant
        System.out.println(str.substring(0, 3)); // Compliant
        System.out.println(str.substring(1, 4)); // Compliant
    }
}
