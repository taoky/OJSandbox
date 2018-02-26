.PHONY: all clean

all: main

CC := gcc
CFLAGS := -O3 -Wall -s
LDFLAGS := -lseccomp
OBJS = main.o secrules.o util.o
CONFIG = config.h

CFG_TL_MARGIN ?= 50

main: $(CONFIG) $(OBJS)
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

config.h:
	echo "#define GITCOMMIT_STR \"`git rev-parse HEAD | head -c8`\"" > $@
	echo "#define GITVERSION_STR \"`git rev-list --count --all`\"" >> $@
	echo "#define TL_MARGIN $(CFG_TL_MARGIN)" >> $@

clean:
	rm -f main $(OBJS) $(CONFIG)
