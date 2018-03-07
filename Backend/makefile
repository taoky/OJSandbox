.PHONY: all clean

CC := gcc
CFLAGS := -std=c99 -O3 -Wall -s
LDFLAGS := -lseccomp -lcap

CONFIG = config.h
OBJS = main.o secrules.o util.o
BIN = safeJudger

all: $(BIN)

$(BIN): $(CONFIG) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

config.h:
	@echo "Building OJSandbox backend (version `git rev-list --count HEAD`, commit `git rev-parse HEAD | head -c7`)"
	@echo "Generating $@"
	@echo "#define GITCOMMIT_STR \"`git rev-parse HEAD | head -c7`\"" > $@
	@echo "#define GITVERSION_STR \"`git rev-list --count HEAD`\"" >> $@

clean:
	rm -f $(BIN) $(OBJS) $(CONFIG)
