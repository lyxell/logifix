public class Test {
    String x;
    public void setX(String y) {
        String x = "hello";
        System.out.println(x);
        // used assignment
        x = "hello!";
        System.out.println(x);
        // unused assignment
        x = "world";
        System.out.println(this.x);
        // used assignment, refers to field
        this.x = y;
    }
    public String getX() {
        return x;
    }
}
