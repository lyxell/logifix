.PHONY: sjp

CXXFLAGS = -std=c++17 -O2

all: repair.o

repair.cpp: repair.dl
	souffle --no-warn \
			--generate=$@ \
			--fact-dir=build \
			--output-dir=build \
			repair.dl

sjp:
	$(MAKE) -C sjp
