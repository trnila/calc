CFLAGS = -lm -std=c99
CC = gcc

calc: calc.o
	$(CC) -o $@ $< $(CFLAGS)

calc.o: calc.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f calc calc.o
