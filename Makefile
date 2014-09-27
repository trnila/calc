CFLAGS = -lm -std=c11

calc: calc.o
	gcc $(CFLAGS) -o $@ $<

calc.o: calc.c
	gcc $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -f calc calc.o
