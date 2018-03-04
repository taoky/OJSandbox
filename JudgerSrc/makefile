.PHONY: all clean

# all: safeJudger

CC := gcc
CFLAGS := -O3 -Wall -s
LDFLAGS := -lseccomp -lcap
OBJS = main.o secrules.o util.o
BIN = safeJudger
CONFIG = config.h

CFG_TL_MARGIN ?= 50

all: $(BIN)

$(BIN): $(CONFIG) $(OBJS)
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $^

config.h:
	echo "#define GITCOMMIT_STR \"`git rev-parse HEAD | head -c7`\"" > $@
	echo "#define GITVERSION_STR \"`git rev-list --count --all`\"" >> $@
	echo "#define TL_MARGIN $(CFG_TL_MARGIN)" >> $@

clean:
	rm -f $(BIN) $(OBJS) $(CONFIG)
