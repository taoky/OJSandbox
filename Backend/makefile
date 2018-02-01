.PHONY: all clean

all: main

CC := gcc
CFLAGS := -O3 -Wall -s
LDFLAGS := -lseccomp
OBJS = main.o secrules.o util.o

main: $(OBJS)
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

clean:
	rm -f main $(OBJS)
