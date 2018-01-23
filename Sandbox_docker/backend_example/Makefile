.PHONY: all clean

SRC=.
SOURCE = ./$(wildcard *.c)
OBJ=.
OBJS = $(patsubst %.c, $(OBJ)/%, $(SOURCE))
CC:=gcc
CFLAGS:= ${CFLAGS} -Wall -Werror

all: $(OBJS)

$(OBJ)/%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJS)

