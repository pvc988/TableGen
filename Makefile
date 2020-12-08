OUTFILE = tablegen
OBJS = main.o \
       dictionary.o \
       grammar.o \
       vector.o

CC ?= gcc
CFLAGS = -ggdb -O0
CCLD = $(CC)
CCLDFLAGS =
LIBS =

all: $(OUTFILE)

$(OUTFILE): $(OBJS)
	$(CCLD) $(CCLDFLAGS) $(OBJS) -o $(OUTFILE) $(LIBS)

clean:
	$(RM) $(OUTFILE) $(OBJS)

.PHONY: clean

