
CXXFLAGS = -std=c++17 -O0 -D__EMBEDDED_SOUFFLE__

all: sjp/sjp.o sjp/parser.o repair.o program.o

repair.o: sjp/sjp.h

program.cpp: repair.dl rules/1155.dl
	souffle --generate=$@ repair.dl

.PHONY: sjp/sjp.o sjp/parser.o

sjp/sjp.o:
	$(MAKE) -C sjp sjp.o

sjp/parser.o:
	$(MAKE) -C sjp parser.o

.PHONY: clean

clean:
	rm -rf repair.o program.o
	$(MAKE) -C sjp clean
