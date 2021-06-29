public class Test {
    public static void main(String[] args) {

        String x = "hello world";

        System.out.println(x.substring(4).indexOf('l')); // Noncompliant
        System.out.println(x.indexOf('l', 4) - 4); // Compliant

        System.out.println(x.substring(1).startsWith("ello")); // Noncompliant
        System.out.println(x.startsWith("ello", 1)); // Compliant

    }
}
