CC = $(CROSS_COMPILE)gcc
CFLAGS = -g -Wall -Wextra -Werror

.PHONY: all
all: default

default: aesdsocket.o
	$(CC) $(CFLAGS) -o aesdsocket aesdsocket.o

aesdsocket.o : aesdsocket.c
	$(CC) $(CFLAGS) -c aesdsocket.c

clean:
	rm -f aesdsocket aesdsocket.o