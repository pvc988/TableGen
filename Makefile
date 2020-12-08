OUTFILE = tablegen
OBJS = main.o \
       dictionary.o \
       fsm.o \
       item.o \
       grammar.o \
       production.o \
       state.o \
       symbol.o \
       transition.o \
       vector.o

CC ?= gcc
CCLD = $(CC)
CCLDFLAGS =
LIBS =

all: $(OUTFILE)

$(OUTFILE): $(OBJS)
	$(CCLD) $(CCLDFLAGS) $(OBJS) -o $(OUTFILE) $(LIBS)

clean:
	$(RM) $(OUTFILE) $(OBJS)

.PHONY: clean

