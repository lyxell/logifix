### Test suite for Logifix

The test suite for Logifix is built around real world commits, see
[tests.csv](./tests.csv).

The test runner asserts that the diff produced by Logifix is
byte-to-byte identical to the one produced by the commit.
