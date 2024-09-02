GCC = gcc
CFLAGS = -Wall -Wshadow -pthread
OBJS = bmp.o filter.o

all: publisher blur sharpen

publisher: publisher.o $(OBJS)
	$(GCC) $(CFLAGS) publisher.o $(OBJS) -o publisher

blur: blur.o $(OBJS)
	$(GCC) $(CFLAGS) blur.o $(OBJS) -o blur

sharpen: sharpen.o $(OBJS)
	$(GCC) $(CFLAGS) sharpen.o $(OBJS) -o sharpen

.c.o: 
	$(GCC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o publisher blur sharpen

test: all
	# Aquí puedes agregar pruebas específicas para cada programa
	# Por ejemplo:
	# ./publisher testcases/test.bmp
	# ./blur
	# ./sharpen outputs/test_out.bmp

testmem: all
	valgrind --tool=memcheck --leak-check=summary ./publisher testcases/test.bmp
	valgrind --tool=memcheck --leak-check=summary ./blur
	valgrind --tool=memcheck --leak-check=summary ./sharpen outputs/test_out.bmp
