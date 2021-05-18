TARGET=logifix.a
OBJS=sjp/sjp.a logifix.o program.o
CXXFLAGS = -std=c++17 -O2 -g -Wfatal-errors -fPIC -fno-gnu-unique -D__EMBEDDED_SOUFFLE__
RULE_FILES := $(shell find rules/ -name '*.dl')

SOUFFLE=souffle
ifdef SOUFFLE_PATH
	SOUFFLE=$(SOUFFLE_PATH:%/=%)/src/souffle
	CXXFLAGS+=-I$(SOUFFLE_PATH:%/=%)/src/include
endif

all: $(TARGET)

logifix.cpp: program.dl $(RULE_FILES)
	$(SOUFFLE) --generate=logifix $<

$(TARGET): $(OBJS)
	$(RM) -f $@
	$(AR) rcT $(TARGET) $^

.PHONY: sjp/sjp.a clean

sjp/sjp.a:
	$(MAKE) -C sjp

clean:
	rm -rf $(OBJS) $(TARGET)
	$(MAKE) -C sjp clean
