CC=g++
CFLAGS=-std=c++11 -Wall -pedantic -I../include/

all: parser_combinator

parser_combinator: parcom.o
	$(CC) -o parcom parcom.o parser_combinator.o

parser_combinator.o: parser_combinator/parser_combinator.cpp parser_combinator/parser_combinator.h 
	$(CC) -c $(CFLAGS) parser_combinator/parser_combinator.cpp -o parser_combinator.o

parcom.o: parser_combinatorics.cpp parser_combinator.o
	$(CC) -c $(CFLAGS) parser_combinatorics.cpp -o parcom.o

clean:
	rm -f parcom *.o

