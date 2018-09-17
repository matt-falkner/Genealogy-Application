CC = gcc
CPPFLAGS=
CFLAGS = -Wall -std=c11 -g
LDFLAGS= -L.

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	SHAREDNAME=libparser.dylib 
endif
ifeq ($(UNAME), Linux)
	SHAREDNAME=libparser.so
endif


all: ../$(SHAREDNAME)

../$(SHAREDNAME): LinkedListAPI.o GEDCOMparser.o GEDCOMutilties.o 
	$(CC) -shared -o ../$(SHAREDNAME)  GEDCOMparser.o LinkedListAPI.o  GEDCOMutilities.o 

GEDCOMparser.o: src/GEDCOMparser.c include/GEDCOMparser.h
	$(CC) $(CFLAGS) -c -fpic src/GEDCOMparser.c -Iinclude 
	
GEDCOMutilties.o: src/GEDCOMutilities.c include/GEDCOMutilities.h
	$(CC) $(CFLAGS) -c -fpic src/GEDCOMutilities.c -Iinclude
	
LinkedListAPI.o: src/LinkedListAPI.c include/LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic src/LinkedListAPI.c -Iinclude 

parser: ../$(SHAREDNAME)


clean:
	rm -rf *.o bin/*.so bin/*.dylib *.so ../*.so executable GEDCOMoutput.dat output.GED  Structure.GED toPrint.output
	

