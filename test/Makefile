OUTFILE = test
OBJS = main.o \
       lexer.o \
       parser.o

GRAMMAR = test.lrpt

TG ?= ../tablegen
TGALGO ?= LALR1
CC ?= gcc
CFLAGS ?= -O2 -fomit-frame-pointer
CCLD ?= $(CC)
CCLDFLAGS ?=
LIBS ?=

all: $(OUTFILE) $(GRAMMAR)

$(OUTFILE): $(OBJS)
	$(CCLD) $(CCLDFLAGS) $^ -o $@

clean:
	$(RM) $(OUTFILE) $(OBJS) $(GRAMMAR)

.SUFFIXES: .grm .lrpt

%.lrpt: %.grm
	$(TG) $^ -o $@ -a $(TGALGO)

.PHONY: clean

