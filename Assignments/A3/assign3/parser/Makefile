CC=gcc
CFLAGS = -Wall -g -std=c11

BIN = ./bin/
PARSER = ../parser/
SRC = ./src/
INCLUDE = ./include/

#UNAME Shell Variable
UNAME_S := $(shell uname -s)

# OS X Dependant Variables
ifeq ($(UNAME_S), Darwin)
    LIB := gedcom.dylib
# GNU/Linux Depedant Variables
else ifeq ($(UNAME_S), Linux)
    LIB := gedcom.so
endif

lib: gedcom.o
	$(CC) -shared -o ../$(LIB) GEDCOMparser.o GEDCOMutilities.o LinkedListAPI.o
	rm *.o

gedcom.o: $(SRC)GEDCOMparser.c $(SRC)GEDCOMutilities.c $(SRC)LinkedListAPI.c $(INCLUDE)GEDCOMparser.h $(INCLUDE)GEDCOMutilities.h $(INCLUDE)LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic $(SRC)GEDCOMparser.c $(SRC)GEDCOMutilities.c $(SRC)LinkedListAPI.c -Iinclude

clean:
	rm -rf ../gedcom.dylib
	rm -rf $(BIN)*.o
	rm -rf $(BIN)*.a
