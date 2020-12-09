OUTFILE = tablegen
OBJS = main.o \
       dictionary.o \
       fsm.o \
       item.o \
       grammar.o \
       parsetable.o \
       production.o \
       state.o \
       symbol.o \
       transition.o \
       vector.o

CC ?= gcc
CFLAGS ?= -O2 -fomit-frame-pointer
CCLD ?= $(CC)
CCLDFLAGS ?=
LIBS ?=

all: $(OUTFILE)

$(OUTFILE): $(OBJS)
	$(CCLD) $(CCLDFLAGS) $(OBJS) -o $(OUTFILE) $(LIBS)

test: $(OUTFILE)
	$(MAKE) -C test

clean:
	$(RM) $(OUTFILE) $(OBJS)

.PHONY: clean test

