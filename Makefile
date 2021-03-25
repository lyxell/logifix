.PHONY: run setup_cpp_benchmark

run: build/rewrites.csv
	@cat build/rewrites.csv

setup_benchmark: build/rewriter build/scanner

build/rewriter: repair.dl sjp/parser.dl
	souffle --no-warn --generate=output.cpp --fact-dir=build --output-dir=build repair.dl
	$(CXX) -std=c++17 -O2 output.cpp -o build/rewriter

build/rewrites.csv: repair.dl sjp/parser.dl build/token.facts rules/1155.dl
	souffle --no-warn --fact-dir=build --output-dir=build repair.dl

build/token.facts: build/scanner Example.java
	build/scanner Example.java build/token.facts

build/scanner: sjp/scanner.c
	@mkdir -p build
	re2c -W --input-encoding utf8 -i sjp/scanner.c -o build/scanner_re2c.c
	$(CC) -Wall -O2 -g -std=c99 build/scanner_re2c.c -o $@

.PHONY: clean

clean:
	rm -rf build
