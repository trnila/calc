CFLAGS = -lm -std=c99
CC = gcc
BIN = ./bin
SOURCE = ./src

$(BIN)/calc: $(BIN)/calc.o
	$(CC) -o $@ $< $(CFLAGS)

$(BIN)/calc.o: $(SOURCE)/calc.c
	mkdir -p $(BIN)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(BIN)/calc $(BIN)/*.o
