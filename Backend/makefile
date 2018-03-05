.PHONY: all clean

CC := gcc
CFLAGS := -std=c99 -O3 -Wall -s
LDFLAGS := -lseccomp -lcap

CONFIG = config.h
OBJS = main.o secrules.o util.o
BIN = safeJudger

all: $(BIN)

$(BIN): $(CONFIG) $(OBJS)
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

config.h:
	echo "#define GITCOMMIT_STR \"`git rev-parse HEAD | head -c7`\"" > $@
	echo "#define GITVERSION_STR \"`git rev-list --count HEAD`\"" >> $@
	echo "#define TL_MARGIN 50" >> $@

clean:
	rm -f $(BIN) $(OBJS) $(CONFIG)
