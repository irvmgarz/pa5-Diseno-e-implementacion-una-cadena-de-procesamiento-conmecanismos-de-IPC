CC = gcc
CFLAGS = -Wall -Wshadow

all: ex5 sharpen

ex5: ex5.o bmp.o filter.o
	$(CC) $(CFLAGS) -o ex5 ex5.o bmp.o filter.o

sharpen: sharpen.o bmp.o
	$(CC) $(CFLAGS) -o sharpen sharpen.o bmp.o

ex5.o: ex5.c bmp.h filter.h
	$(CC) $(CFLAGS) -c ex5.c

sharpen.o: sharpen.c bmp.h
	$(CC) $(CFLAGS) -c sharpen.c

bmp.o: bmp.c bmp.h
	$(CC) $(CFLAGS) -c bmp.c

filter.o: filter.c filter.h bmp.h
	$(CC) $(CFLAGS) -c filter.c

clean:
	rm -f ex5 sharpen *.o
