<h1 align="center">
  <img src="https://raw.githubusercontent.com/lyxell/logifix/master/.github/logo.svg" alt="Logifix">
</h1>

Logifix statically detects and automatically fixes bugs and code
smells in Java source code. Logifix is implemented in a
high-performance Datalog dialect that is synthesized into fast
multi-threaded C++ code.

https://user-images.githubusercontent.com/4975941/120327017-9c470280-c2e9-11eb-88d1-163943d550ce.mp4

## Downloading

There are pre-built binaries available for GNU/Linux without any
runtime dependencies, see
[Releases](https://github.com/lyxell/logifix/releases).

## Building

To build from source you will need [CMake](https://cmake.org/), [GNU Bison](https://www.gnu.org/software/bison/) and [re2c](https://re2c.org/).

To download and build the project, perform the following steps:

* `git clone https://github.com/lyxell/logifix`
* `cd logifix && git submodule update --init`
* `mkdir build && cd build && cmake .. && cmake --build .`

## Available fixes

| ID                                                     | Category   | Description                                                                     |
|--------------------------------------------------------|------------|---------------------------------------------------------------------------------|
| [S1116](https://rules.sonarsource.com/java/RSPEC-1116) | Code Smell | Empty statements should be removed                                              |
| [S1125](https://rules.sonarsource.com/java/RSPEC-1125) | Code Smell | Boolean literals should not be redundant                                        |
| [S1132](https://rules.sonarsource.com/java/RSPEC-1132) | Code Smell | Strings literals should be placed on the left side when checking for equality   |
| [S1153](https://rules.sonarsource.com/java/RSPEC-1153) | Code Smell | String.valueOf() should not be appended to a String                             |
| [S1155](https://rules.sonarsource.com/java/RSPEC-1155) | Code Smell | Collection.isEmpty() should be used to test for emptiness                       |
| [S1217](https://rules.sonarsource.com/java/RSPEC-1217) | Bug        | Thread.run() should not be called directly                                      |
| [S1481](https://rules.sonarsource.com/java/RSPEC-1481) | Code Smell | Unused local variables should be removed                                        |
| [S1596](https://rules.sonarsource.com/java/RSPEC-1596) | Code Smell | Collections.EMPTY_LIST, EMPTY_MAP, and EMPTY_SET should not be used             |
| [S1602](https://rules.sonarsource.com/java/RSPEC-1602) | Code Smell | Lambdas containing only one statement should not nest this statement in a block |
| [S1764](https://rules.sonarsource.com/java/RSPEC-1764) | Bug        | Identical expressions should not be used on both sides of a binary operator     |
| [S2095](https://rules.sonarsource.com/java/RSPEC-2095) | Bug        | Resources should be closed                                                      |
| [S2111](https://rules.sonarsource.com/java/RSPEC-2111) | Bug        | BigDecimal(double) should not be used                                           |
| [S2114](https://rules.sonarsource.com/java/RSPEC-2114) | Bug        | Collections should not be passed as arguments to their own methods              |
| [S2121](https://rules.sonarsource.com/java/RSPEC-2121) | Bug        | Silly String operations should not be made                                      |
| [S2159](https://rules.sonarsource.com/java/RSPEC-2159) | Bug        | Silly equality checks should not be made                                        |
| [S2204](https://rules.sonarsource.com/java/RSPEC-2204) | Bug        | equals() should not be used to test the values of Atomic classes                |
| [S2225](https://rules.sonarsource.com/java/RSPEC-2225) | Bug        | toString() and clone() methods should not return null                           |
| [S2259](https://rules.sonarsource.com/java/RSPEC-2259) | Bug        | Null pointers should not be dereferenced                                        |
| [S3984](https://rules.sonarsource.com/java/RSPEC-3984) | Bug        | Exceptions should not be created without being thrown                           |
| [S4087](https://rules.sonarsource.com/java/RSPEC-4087) | Code Smell | close() calls should not be redundant                                           |
| [S4635](https://rules.sonarsource.com/java/RSPEC-4635) | Code Smell | String offset-based methods should be preferred for finding substrings          |
| [S4973](https://rules.sonarsource.com/java/RSPEC-4973) | Bug        | Strings and Boxed types should be compared using equals()                       |
