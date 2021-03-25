## datalog-repair

This proof of concept parses the file [Example.java](https://github.com/lyxell/datalog-repair/blob/master/Example.java)
and then uses the rule [1155.dl](https://github.com/lyxell/datalog-repair/blob/master/rules/1155.dl)
to figure out that there is a possible rewrite, namely `x.size() == 0` → `x.isEmpty()`.

Lexing, parsing, and finding the rewrite all completes in 0.015
s (15 milliseconds) on an Intel(R) Core(TM) i5-3320M CPU. This time
includes disk I/O (2 file writes and 2 file reads).

It uses my library [https://github.com/lyxell/sjp](https://github.com/lyxell/sjp)
to build the ASTs.

### Running the benchmark

Dependencies:

* [re2c](https://github.com/skvadrik/re2c) (lexer generator)
* clang or gcc
* [Soufflé](https://github.com/souffle-lang/souffle)
* make

Steps:

* `make setup_benchmark`
* `time build/scanner Example.java build/token.facts` (lexing)
* `time build/rewriter` (parsing and find rewrite)
* `cat build/rewrites.csv`
