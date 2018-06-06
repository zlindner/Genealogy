CC = gcc
CFLAGS = -Wall -g -std=c11

BIN = ./bin/
SRC = ./src/

A1:
	$(CC) $(CFLAGS) $(SRC)A1.c $(SRC)GEDCOMparser.c $(SRC)GEDCOMutilities.c $(SRC)LinkedListAPI.c -Iinclude -o $(BIN)A1

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
