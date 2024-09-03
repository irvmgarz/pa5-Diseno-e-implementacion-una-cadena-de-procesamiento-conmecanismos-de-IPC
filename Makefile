CC = gcc
CFLAGS = -Wall -Wshadow
TARGET = ex5

all: $(TARGET)

$(TARGET): ex5.o bmp.o filter.o
	$(CC) $(CFLAGS) -o $(TARGET) ex5.o bmp.o filter.o

ex5.o: ex5.c bmp.h
	$(CC) $(CFLAGS) -c ex5.c

bmp.o: bmp.c bmp.h
	$(CC) $(CFLAGS) -c bmp.c

filter.o: filter.c filter.h bmp.h
	$(CC) $(CFLAGS) -c filter.c

clean:
	rm -f $(TARGET) *.o
