CFLAGS = -lm -std=c99 -Wall
CC = gcc
BIN = ./bin
SOURCE = ./src

all: prepare $(BIN)/tests $(BIN)/cli

$(BIN)/tests: $(BIN)/tests.o $(BIN)/calc.o
	$(CC) -o $@ $^ $(CFLAGS)

$(BIN)/cli: $(BIN)/cli.o $(BIN)/calc.o
	$(CC) -o $@ $^ $(CFLAGS)

$(BIN)/%.o: $(SOURCE)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean prepare

clean:
	rm -f $(BIN)/calc $(BIN)/*.o

prepare:
		@mkdir -p $(BIN)
test:
		./bin/tests
