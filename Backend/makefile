.PHONY: all clean

CC := gcc
CFLAGS := -O3 -Wall -s
LDFLAGS := -lseccomp

CONFIG = config.h
OBJS = main.o secrules.o util.o
BIN = safeJudger

CFG_TL_MARGIN ?= 50

all: $(BIN)

$(BIN): $(CONFIG) $(OBJS)
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

config.h:
	echo "#define GITCOMMIT_STR \"`git rev-parse HEAD | head -c8`\"" > $@
	echo "#define GITVERSION_STR \"`git rev-list --count --all`\"" >> $@
	echo "#define TL_MARGIN $(CFG_TL_MARGIN)" >> $@

clean:
	rm -f $(BIN) $(OBJS) $(CONFIG)
