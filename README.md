<h1 align="center">
  <img src="https://i.imgur.com/7o8qjPX.png" alt="Logifix">
</h1>

Logifix automatically repairs SonarQube static analysis
violations for Java. Logifix is implemented in a high-performance
Datalog dialect that is synthesized into fast multi-threaded C++
code.

Logifix is built to be easy to integrate into IDEs, CLI tools,
CI pipelines and bots.

<img src="https://i.imgur.com/Ip28qxt.gif" alt="Logifix demo">

## Targeted rules

* [S1125 - Boolean literals should not be redundant](https://github.com/lyxell/logifix/blob/master/rules/1125.dl)
* [S1132 - Strings literals should be placed on the left side when checking for equality](https://github.com/lyxell/logifix/blob/master/rules/1132.dl)
* [S1155 - Collection.isEmpty() should be used to test for emptiness](https://github.com/lyxell/logifix/blob/master/rules/1155.dl)
* [S1596 - Collections.EMPTY_LIST, EMPTY_MAP, and EMPTY_SET should not be used](https://github.com/lyxell/logifix/blob/master/rules/1596.dl)
* [S2111 - BigDecimal(double) should not be used](https://github.com/lyxell/logifix/blob/master/rules/2111.dl)
* [S2204 - .equals() should not be used to test the values of Atomic classes](https://github.com/lyxell/logifix/blob/master/rules/2204.dl)
* [S2272 - Iterator.next() methods should throw NoSuchElementException](https://github.com/lyxell/logifix/blob/master/rules/2272.dl)
* [S2293 - The diamond operator ("<>") should be used](https://github.com/lyxell/logifix/blob/master/rules/2293.dl)
* [S3984 - Exceptions should not be created without being thrown](https://github.com/lyxell/logifix/blob/master/rules/3984.dl)
* [S4973 - Strings and Boxed types should be compared using equals()](https://github.com/lyxell/logifix/blob/master/rules/4973.dl)
