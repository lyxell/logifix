class Test {
    void test() {
        String str = "hello world";
        System.out.println(str.substring(0));
        System.out.println(str.substring(0, str.length()));
        System.out.println(str.substring(str.length()));
        System.out.println(str.substring(3, str.length()));
        System.out.println(str.substring(0, 3));
        System.out.println(str.substring(1, 4));
    }
}
