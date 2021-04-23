
CXXFLAGS = -std=c++17 -O2 -D__EMBEDDED_SOUFFLE__

all: sjp repair.o program.o

repair.cpp: repair.dl rules/1155.dl sjp/parser.dl
	souffle --generate=$@ repair.dl

.PHONY: sjp

sjp:
	$(MAKE) -C sjp sjp.o

.PHONY: clean

clean:
	rm -rf repair.o program.o
	$(MAKE) -C sjp clean
