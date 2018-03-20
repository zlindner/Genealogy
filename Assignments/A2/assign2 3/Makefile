CC = gcc
CFLAGS = -Wall -g -std=c11

BIN = ./bin/
SRC = ./src/

all: list parser

list:
	$(CC) $(CFLAGS) -c -o $(BIN)LinkedListAPI.o $(SRC)LinkedListAPI.c -Iinclude
	ar rcs $(BIN)liblist.a $(BIN)LinkedListAPI.o

parser:
	$(CC) $(CFLAGS) -c -o $(BIN)GEDCOMparser.o $(SRC)GEDCOMparser.c -Iinclude
	$(CC) $(CFLAGS) -c -o $(BIN)GEDCOMutilities.o $(SRC)GEDCOMutilities.c -Iinclude
	ar rcs $(BIN)libparser.a $(BIN)GEDCOMparser.o $(BIN)GEDCOMutilities.o

clean:
	rm -rf $(BIN)*.o
	rm -rf $(BIN)*.a
