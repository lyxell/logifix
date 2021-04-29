TARGET=datalog-repair.a
OBJS=sjp/sjp.o sjp/parser.o repair.o program.o
CXXFLAGS = -std=c++17 -O0 -D__EMBEDDED_SOUFFLE__

all: $(TARGET)

repair.o: sjp/sjp.h

program.cpp: repair.dl rules/1155.dl
	souffle --generate=$@ repair.dl

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
