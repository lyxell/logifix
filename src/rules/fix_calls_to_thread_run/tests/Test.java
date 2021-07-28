public class Test {
    void test() {
        Runnable runnable = null;

        Thread myThread = new Thread(runnable);
        myThread.run();

        Thread myThread2 = new Thread(runnable);
        myThread2.start();

    }
}
