CFLAGS=-Wall -Werror -pedantic -ansi -lpthread

all:
	$(CC) $(CFLAGS) identd.c -o identd

clean:
	rm identd
