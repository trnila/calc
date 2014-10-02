CFLAGS = -lm -std=c99 -Wall
CC = gcc
BIN = ./bin
SOURCE = ./src

all: prepare $(BIN)/tests $(BIN)/cli

$(BIN)/tests: $(BIN)/tests.o $(BIN)/calc.o
	$(CC) -g -o $@ $^ $(CFLAGS)

$(BIN)/cli: $(BIN)/cli.o $(BIN)/calc.o
	$(CC) -g -o $@ $^ $(CFLAGS) -lreadline

$(BIN)/%.o: $(SOURCE)/%.c
	$(CC) -g -c -o $@ $< $(CFLAGS)

.PHONY: clean prepare

clean:
	rm -f $(BIN)/calc $(BIN)/*.o

prepare:
		@mkdir -p $(BIN)
test:
		./bin/tests
