TARGET=logifix.a
OBJS=sjp/parser.o sjp/lexer.o sjp/program.o program.o repair.o
CXXFLAGS = -std=c++17 -O2 -g -Wfatal-errors -fPIC -fno-gnu-unique -D__EMBEDDED_SOUFFLE__
RULE_FILES := $(shell find rules/ -name '*.dl')

SOUFFLE=souffle
ifdef SOUFFLE_PATH
	SOUFFLE=$(SOUFFLE_PATH:%/=%)/src/souffle
	CXXFLAGS+=-I$(SOUFFLE_PATH:%/=%)/src/include
endif

all: $(TARGET)

logifix.o: sjp/sjp.h

program.o: program.dl $(RULE_FILES)
	$(SOUFFLE) --generate=logifix $<
	$(CXX) $(CXXFLAGS) logifix.cpp -c -o $@
	rm logifix.cpp

$(TARGET): $(OBJS)
	$(AR) -rc $(TARGET) $^

.PHONY: sjp/program.o sjp/parser.o sjp/lexer.o clean

sjp/parser.o:
	$(MAKE) -C sjp parser.o

sjp/lexer.o:
	$(MAKE) -C sjp lexer.o

sjp/program.o:
	$(MAKE) -C sjp program.o

clean:
	rm -rf $(OBJS) $(TARGET)
	$(MAKE) -C sjp clean
