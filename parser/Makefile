CC = gcc
CPPFLAGS=
CFLAGS = -Wall -std=c11 -g
LDFLAGS= -L.


all: ../libparser.so 

../libparser.so: LinkedListAPI.o GEDCOMparser.o GEDCOMutilties.o 
	$(CC) -shared -o ../libparser.so  GEDCOMparser.o LinkedListAPI.o  GEDCOMutilities.o 


GEDCOMparser.o: src/GEDCOMparser.c include/GEDCOMparser.h
	$(CC) $(CFLAGS) -c -fpic src/GEDCOMparser.c -Iinclude 
	
GEDCOMutilties.o: src/GEDCOMutilities.c include/GEDCOMutilities.h
	$(CC) $(CFLAGS) -c -fpic src/GEDCOMutilities.c -Iinclude
	
LinkedListAPI.o: src/LinkedListAPI.c include/LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic src/LinkedListAPI.c -Iinclude 

parser: ../libparser.so


clean:
	rm -rf *.o bin/*.so bin/*.dylib *.so ../*.so executable GEDCOMoutput.dat output.GED  Structure.GED toPrint.output

run1: 
	./executable testing/simple.ged
	
run2:
	./executable testing/royal92.ged

run3: 
	./executable testing/shakespeare.ged
	

