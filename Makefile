OBJDIR=.
BINDIR=.
SRCDIR=.
CFLAGS=-c -std=c99 -Wall -pedantic -g -D_GNU_SOURCE
LDFLAGS=-lncurses -lm

all: main test

main: main.o log.o util.o
	gcc $^ $(LDFLAGS) -o $@

test: test.o log.o util.o
	gcc $^ $(LDFLAGS) -o $@

%.o: %.c
	gcc $(CFLAGS) $< -o $@

clean:
	/bin/rm -rf *.o *.d test main vgcore* *.log *.exe

tags:
	ctags -R *

.PHONY: clean tags

