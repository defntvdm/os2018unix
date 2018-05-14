CC=gcc
CFLAGS=-std=c99
SRC=main.c\
	array.c

all: initd

.PHONY: initd
initd: $(SRC)
	$(CC) -o $@ $^ $(CFLAGS)
