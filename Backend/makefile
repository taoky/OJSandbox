.PHONY: all clean

all: main

CFLAGS := -O3 -Wall -s
LDFLAGS := -lseccomp
OBJS = main.o secrules.o util.o

main: $(OBJS)
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^

%.o: %.c
	${CC} ${CFLAGS} -o $@ $^

clean:
	rm -f main $(OBJS)
