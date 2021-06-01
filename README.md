<h1 align="center">
  <img src="https://raw.githubusercontent.com/lyxell/logifix/master/.github/logo.svg" alt="Logifix">
</h1>

Logifix automatically fixes SonarQube static analysis
violations for Java. Logifix is implemented in a high-performance
Datalog dialect that is synthesized into fast multi-threaded C++
code.

Logifix is built to be easy to integrate into IDEs, CLI tools,
CI pipelines and bots.

https://user-images.githubusercontent.com/4975941/119820849-3413ad80-bef2-11eb-8954-e23f9a5fdde8.mp4

## Building

To build from source you will need [CMake](https://cmake.org/), [GNU Bison](https://www.gnu.org/software/bison/) and [re2c](https://re2c.org/).

To download and build the project, perform the following steps:

* `git clone https://github.com/lyxell/logifix`
* `cd logifix && git submodule update --init`
* `mkdir build && cd build && cmake .. && cmake --build .`

## Available fixes

### Boolean literals should not be redundant

Redundant Boolean literals should be removed from expressions to improve readability.

* SonarQube ID: [S1125](https://rules.sonarsource.com/java/RSPEC-1125)

### Strings literals should be placed on the left side when checking for equality

It is preferable to place string literals on the left-hand side of an equals() or equalsIgnoreCase() method call.

This prevents null pointer exceptions from being raised, as a string literal can never be null by definition.

* SonarQube ID: [S1132](https://rules.sonarsource.com/java/RSPEC-1132)

### Collection.isEmpty() should be used to test for emptiness

Using `Collection.size()` to test for emptiness works, but using `Collection.isEmpty()` makes the code more readable and can be more performant. The time complexity of any `isEmpty()` method implementation should be O(1) whereas some implementations of `size()` can be O(n).

* SonarQube ID: [S1155](https://rules.sonarsource.com/java/RSPEC-1155)

### Collections.EMPTY_LIST, EMPTY_MAP, and EMPTY_SET should not be used

Since the introduction of generics in Java 5, the use of generic types such as List<String> is recommended over the use of raw ones such as List. Assigning a raw type to a generic one is not type safe, and will generate a warning. The old EMPTY_... fields of the Collections class return raw types, whereas the newer empty...() methods return generic ones.

* SonarQube ID: [S1596](https://rules.sonarsource.com/java/RSPEC-1596)

### BigDecimal(double) should not be used
  
Because of floating point imprecision, you're unlikely to get the value you expect from the BigDecimal(double) constructor.

Instead, you should use BigDecimal.valueOf, which uses a string under the covers to eliminate floating point rounding errors, or the constructor that takes a String argument.
 
* SonarQube ID: [S2111](https://rules.sonarsource.com/java/RSPEC-2111)

### Lambdas containing only one statement should not nest this statement in a block

There are two ways to write lambdas that contain single statement, but one is definitely more compact and readable than the other.

* SonarQube ID: [S1602](https://rules.sonarsource.com/java/RSPEC-1602)

### Resources should be closed

Connections, streams, files, and other classes that implement the Closeable interface or its super-interface, AutoCloseable, needs to be closed after use. Further, that close call must be made in a finally block otherwise an exception could keep the call from being made. Preferably, when class implements AutoCloseable, resource should be created using "try-with-resources" pattern and will be closed automatically.

* SonarQube ID: [S2095](https://rules.sonarsource.com/java/RSPEC-2095)

### .equals() should not be used to test the values of Atomic classes

`AtomicInteger`, and `AtomicLong` extend `Number`, but they're distinct from `Integer` and `Long` and should be handled differently. `AtomicInteger` and `AtomicLong are` designed to support lock-free, thread-safe programming on single variables. As such, an `AtomicInteger` will only ever be "equal" to itself. Instead, you should .get() the value and make comparisons on it.

This applies to all the atomic, seeming-primitive wrapper classes: `AtomicInteger`, `AtomicLong`, and `AtomicBoolean`.
  
* SonarQube ID: [S2204](https://rules.sonarsource.com/java/RSPEC-2204)

### Iterator.next() methods should throw NoSuchElementException
  
By contract, any implementation of the `java.util.Iterator.next()` method should throw a `NoSuchElementException` exception when the iteration has no more elements. Any other behavior when the iteration is done could lead to unexpected behavior for users of this `Iterator`.
  
* SonarQube ID: [S2272](https://rules.sonarsource.com/java/RSPEC-2272)
  
### Exceptions should not be created without being thrown
  
Creating a new `Throwable` without actually throwing it is useless and is probably due to a mistake.
  
* SonarQube ID: [S3984](https://rules.sonarsource.com/java/RSPEC-3984)
  
### Strings and Boxed types should be compared using equals()
  
It's almost always a mistake to compare two instances of `java.lang.String` or boxed types like `java.lang.Integer` using reference equality == or !=, because it is not comparing actual value but locations in memory.
  
* SonarQube ID: [S4973](https://rules.sonarsource.com/java/RSPEC-4973)
