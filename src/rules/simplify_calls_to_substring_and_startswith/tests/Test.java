public class Test {
    public static void main(String[] args) {
        String x = "hello world";
        System.out.println(x.substring(1).startsWith("ello"));
        // ok
        System.out.println(x.startsWith("ello", 1));
    }
}
