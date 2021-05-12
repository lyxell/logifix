# SquareLog

SquareLog automatically repairs SonarQube static analysis
violations for Java. SquareLog is implemented in a high-performance
Datalog dialect that is synthesized into fast multi-threaded C++
code.

SquareLog is built to be easy to integrate into IDEs, CLI tools,
CI pipelines and bots.

## Demo

### Strings and Boxed types should be compared using equals()

![](https://i.imgur.com/OT5nvEp.gif)

* [SonarJava rule S4973](https://rules.sonarsource.com/java/RSPEC-4973)

### Exceptions should not be created without being thrown

![](https://i.imgur.com/mbYgieq.gif)

* [SonarJava rule S3984](https://rules.sonarsource.com/java/RSPEC-3984)
