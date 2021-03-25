.PHONY: run

run: build/rewrites.csv
	@cat build/rewrites.csv

build/rewrites.csv: repair.dl sjp/parser.dl build/token.facts rules/1155.dl
	souffle --fact-dir=build --output-dir=build repair.dl

build/token.facts: build/scanner Example.java
	build/scanner Example.java > $@

build/scanner: sjp/scanner.c
	@mkdir -p build
	re2c -W --input-encoding utf8 -i sjp/scanner.c -o build/scanner_re2c.c
	gcc -Wall -O2 -g -std=c99 build/scanner_re2c.c -o $@

.PHONY: clean

clean:
	rm -rf build
