.PHONY: all clean

all: main

CC := gcc
CFLAGS := -O3 -Wall -s
LDFLAGS := -lseccomp
OBJS = main.o secrules.o util.o

main: gitversion.h $(OBJS)
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

gitversion.h:
	echo "#define GITCOMMIT_STR \"`git rev-parse HEAD | head -c8`\"" > $@
	echo "#define GITVERSION_STR \"`git rev-list --count --all`\"" >> $@

clean:
	rm -f main $(OBJS) gitversion.h
