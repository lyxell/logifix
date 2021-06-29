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

### Fix broken null checks

* PMD ID: [BrokenNullCheck](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#brokennullcheck)
* SonarSource ID: [S2259](https://rules.sonarsource.com/java/RSPEC-2259)

<ul> </ul>


### Fix calls to Thread.run

* PMD ID: [DontCallThreadRun](https://pmd.github.io/latest/pmd_rules_java_multithreading.html#dontcallthreadrun)
* SonarSource ID: [S1217](https://rules.sonarsource.com/java/RSPEC-1217)

<ul> </ul>


### Fix comparisons of atomic classes

* PMD ID: N/A
* SonarSource ID: [S2204](https://rules.sonarsource.com/java/RSPEC-2204)

<ul> </ul>


### Fix comparisons of objects with null

* PMD ID: [EqualsNull](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#equalsnull)
* SonarSource ID: [S2159](https://rules.sonarsource.com/java/RSPEC-2159)

<ul> </ul>


### Fix null pointer exceptions by changing the order of arguments in string comparison

* PMD ID: [LiteralsFirstInComparison](https://pmd.github.io/latest/pmd_rules_java_bestpractices.html#literalsfirstincomparisons)
* SonarSource ID: [S1132](https://rules.sonarsource.com/java/RSPEC-1132)

<ul> </ul>


### Fix null returns in toString methods

* PMD ID: N/A
* SonarSource ID: [S2225](https://rules.sonarsource.com/java/RSPEC-2225)

<ul> </ul>


### Fix references to Collections.EMPTY_LIST, EMPTY_MAP and EMPTY_SET

* PMD ID: N/A
* SonarSource ID: [S1596](https://rules.sonarsource.com/java/RSPEC-1596)

<ul> </ul>


### Remove redundant calls to close

* PMD ID: N/A
* SonarSource ID: [S4087](https://rules.sonarsource.com/java/RSPEC-4087)

<ul> </ul>


### Improve precision of calls to BigDecimal

* PMD ID: [AvoidDecimalLiteralsInBigDecimalConstructor](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#avoiddecimalliteralsinbigdecimalconstructor)
* SonarSource ID: [S2111](https://rules.sonarsource.com/java/RSPEC-2111)

<ul> </ul>


### Remove empty finally blocks

* PMD ID: [EmptyFinallyBlock](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#emptyfinallyblock)
* SonarSource ID: N/A

<ul> </ul>


### Remove empty nested blocks

* PMD ID: [EmptyStatementBlock](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#emptystatementblock)
* SonarSource ID: N/A

<ul> </ul>


### Remove empty statements

* PMD ID: [EmptyStatementNotInLoop](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#emptystatementnotinloop)
* SonarSource ID: [S1116](https://rules.sonarsource.com/java/RSPEC-1116)

<ul> </ul>


### Remove empty try blocks

* PMD ID: [EmptyTryBlock](https://pmd.github.io/pmd-6.36.0/pmd_rules_java_errorprone.html#emptytryblock)
* SonarSource ID: N/A

<ul> </ul>


### Remove repeated unary operators

* PMD ID: [AvoidMultipleUnaryOperators](https://pmd.github.io/latest/pmd_rules_java_errorprone.html#avoidmultipleunaryoperators)
* SonarSource ID: [S2761](https://rules.sonarsource.com/java/RSPEC-2761)

<ul> </ul>


### Remove unnecessary calls to String.valueOf

* PMD ID: [UselessStringValueOf](https://pmd.github.io/latest/pmd_rules_java_performance.html#uselessstringvalueof)
* SonarSource ID: [S1153](https://rules.sonarsource.com/java/RSPEC-1153)

<ul> </ul>


### Remove unnecessary variable declarations before return statements

* PMD ID: [UnnecessaryLocalBeforeReturn](https://pmd.github.io/latest/pmd_rules_java_codestyle.html#unnecessarylocalbeforereturn)
* SonarSource ID: [S1488](https://rules.sonarsource.com/java/RSPEC-1488)

<ul> </ul>


### Remove unused local variables

* PMD ID: [UnusedLocalVariable](https://pmd.github.io/latest/pmd_rules_java_bestpractices.html#unusedlocalvariable)
* SonarSource ID: [S1481](https://rules.sonarsource.com/java/RSPEC-1481)

<ul> </ul>


### Remove unused private fields

* PMD ID: [UnusedPrivateField](https://pmd.github.io/latest/pmd_rules_java_bestpractices.html#unusedprivatefield)
* SonarSource ID: [S1068](https://rules.sonarsource.com/java/RSPEC-1068)

<ul> </ul>


### Simplify boolean expressions

* PMD ID: [SimplifyBooleanExpressions](https://pmd.github.io/latest/pmd_rules_java_design.html#simplifybooleanexpressions)
* SonarSource ID: [S1125](https://rules.sonarsource.com/java/RSPEC-1125)

<ul> </ul>


### Simplify calls to Collection.removeAll

* PMD ID: N/A
* SonarSource ID: [S2114](https://rules.sonarsource.com/java/RSPEC-2114)

<ul> </ul>


### Simplify calls to String.substring

* PMD ID: N/A
* SonarSource ID: [S2121](https://rules.sonarsource.com/java/RSPEC-2121)

<ul> </ul>


### Simplify calls to String.substring and String.startsWith

* PMD ID: N/A
* SonarSource ID: [S4635](https://rules.sonarsource.com/java/RSPEC-4635)

<ul> </ul>


### Simplify code using Collection.isEmpty

* PMD ID: [UseCollectionIsEmpty](https://pmd.github.io/latest/pmd_rules_java_bestpractices.html#usecollectionisempty)
* SonarSource ID: [S1155](https://rules.sonarsource.com/java/RSPEC-1155)

<ul> </ul>


### Simplify code using Map.computeIfAbsent

* PMD ID: N/A
* SonarSource ID: [S3824](https://rules.sonarsource.com/java/RSPEC-3824)

<ul> </ul>


### Simplify inverted boolean expressions

* PMD ID: [LogicInversion](https://pmd.github.io/latest/pmd_rules_java_design.html#logicinversion)
* SonarSource ID: [S1940](https://rules.sonarsource.com/java/RSPEC-1940)

<ul> </ul>


### Simplify lambdas containing a block with only one statement

* PMD ID: N/A
* SonarSource ID: [S1602](https://rules.sonarsource.com/java/RSPEC-1602)

<ul> </ul>


### Simplify return of boolean expressions

* PMD ID: [SimplifyBooleanReturns](https://pmd.github.io/latest/pmd_rules_java_design.html#simplifybooleanreturns)
* SonarSource ID: [S1126](https://rules.sonarsource.com/java/RSPEC-1126)

<ul> </ul>


### Simplify ternary conditional expressions

* PMD ID: [SimplifiedTernary](https://pmd.github.io/latest/pmd_rules_java_design.html#simplifiedternary)
* SonarSource ID: [S1125](https://rules.sonarsource.com/java/RSPEC-1125)

<ul> </ul>

