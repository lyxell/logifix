## datalog-repair

This proof of concept parses the file [Example.java](https://github.com/lyxell/datalog-repair/blob/master/Example.java)
and then uses the rule [1155.dl](https://raw.githubusercontent.com/lyxell/datalog-repair/master/rules/1155.dl)
to figure out that there is a possible rewrite, namely `x.size() == 0` -> `x.isEmpty()`.

Lexing, parsing, and finding the rewrite for rule 1155 completes in 0.015
s (15 milliseconds) on an Intel(R) Core(TM) i5-3320M CPU. This time
includes disk I/O (2 file writes and 2 file reads).
