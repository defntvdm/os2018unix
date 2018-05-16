CC=gcc
CCFLAGS=-std=c99
SRC=main.c \
	lock.c

FOR_TEST_SRC=main.c \
			 bad_lock.c

all: lock

.PHONY: lock
lock: $(SRC)
	$(CC) -o $@ $^ $(CCFLAGS)

.PHONY: for_test
for_test: $(FOR_TEST_SRC)
	$(CC) -o $@ $^ $(CCFLAGS)

