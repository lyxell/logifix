public class Test {
    String x;
    public void setX(String y) {
        String x = "hello";
        System.out.println(x);
        // unused assignment, should be removed
        x = "world";
        // used assignment, refers to field
        this.x = y;
    }
    public String getX() {
        return x;
    }
}
