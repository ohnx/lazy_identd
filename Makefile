CFLAGS=-Wall -Werror -pedantic -ansi -lpthread -pthread

all:
	$(CC) $(CFLAGS) identd.c -o identd

clean:
	rm identd
