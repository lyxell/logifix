TARGET=datalog-repair.a
OBJS=sjp/sjp.o sjp/parser.o repair.o program.o functors.o
CXXFLAGS = -std=c++17 -O2 -fPIC -fno-gnu-unique -D__EMBEDDED_SOUFFLE__
RULE_FILES := $(shell find rules/ -name '*.dl')

SOUFFLE=souffle
ifdef SOUFFLE_PATH
	SOUFFLE=$(SOUFFLE_PATH:%/=%)/src/souffle
	CXXFLAGS+=-I$(SOUFFLE_PATH:%/=%)/src/include
endif

all: $(TARGET)

repair.o: sjp/sjp.h

program.cpp: repair.dl $(RULE_FILES)
	$(SOUFFLE) --generate=$@ repair.dl

$(TARGET): $(OBJS)
	$(AR) -rc $(TARGET) $^

.PHONY: sjp/sjp.o sjp/parser.o clean
sjp/sjp.o:
	$(MAKE) -C sjp sjp.o

sjp/parser.o:
	$(MAKE) -C sjp parser.o

clean:
	rm -rf $(OBJS) $(TARGET)
	$(MAKE) -C sjp clean
