.PHONY: all clean

all: judger

CC := gcc
CFLAGS := -O3 -Wall -s
LDFLAGS := -lseccomp
OBJS = main.o secrules.o util.o

judger: $(OBJS)
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

clean:
	rm -f judger $(OBJS)
