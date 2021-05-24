TOOL=cli/logifix
ARCHIVE=logifix.a
OBJS=sjp/sjp.a logifix.o program.o
CXXFLAGS = -std=c++17 -O2 -Wfatal-errors -DDEBUG -fPIC -fno-gnu-unique -D__EMBEDDED_SOUFFLE__
RULE_FILES := $(shell find rules/ -name '*.dl')

SOUFFLE=souffle
ifdef SOUFFLE_PATH
	SOUFFLE=$(SOUFFLE_PATH:%/=%)/src/souffle
	CXXFLAGS+=-I$(SOUFFLE_PATH:%/=%)/src/include
endif

all: $(TOOL) $(ARCHIVE)

cli/logifix: logifix.o program.o sjp/sjp.a cli/main.cpp
	$(CXX) $(CXXFLAGS) $^ -lgit2 -o $@


logifix.cpp: program.dl $(RULE_FILES)
	$(SOUFFLE) --generate=logifix $<

$(ARCHIVE): $(OBJS)
	$(RM) -f $@
	$(AR) rcT $(ARCHIVE) $^

.PHONY: sjp/sjp.a clean

sjp/sjp.a:
	$(MAKE) -C sjp

test: cli/logifix
	cd tests && ./download_tests.sh
	cd tests && ./run_tests.sh ../cli/logifix

clean:
	rm -rf $(OBJS) $(ARCHIVE) $(TOOL)
	$(MAKE) -C sjp clean
