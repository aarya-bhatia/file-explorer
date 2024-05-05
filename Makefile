CFLAGS=-c -std=c99 -Wall -pedantic -g -D_GNU_SOURCE
LDFLAGS=-lncurses -lm

MAIN_SRC=src/main.c src/log.c src/util.c src/file.c src/sort.c
TEST_SRC=src/test.c src/log.c src/util.c src/file.c src/sort.c

MAIN_OBJ=$(MAIN_SRC:src/%.c=obj/%.o)
TEST_OBJ=$(TEST_SRC:src/%.c=obj/%.o)

all: bin/main bin/test

bin/main: $(MAIN_OBJ)
	mkdir -p bin
	gcc $^ $(LDFLAGS) -o $@

bin/test: $(TEST_OBJ)
	mkdir -p bin
	gcc $^ $(LDFLAGS) -o $@

obj/%.o: src/%.c
	mkdir -p obj
	gcc $(CFLAGS) $< -o $@

clean:
	rm -rf obj/ bin/ vgcore* *.log

tags:
	ctags -R src

.PHONY: clean tags

