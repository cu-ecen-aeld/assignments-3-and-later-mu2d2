# Makefile for aesdsocket server application
# This was partially modified with the help of ChatGPT
#Link to chat session: https://chatgpt.com/share/6990b762-3dd8-8008-b70d-03f9fc138aca
CC = $(CROSS_COMPILE)gcc
CFLAGS = -g -Wall -Wextra -Werror

.PHONY: all default clean
all: default

default: aesdsocket

aesdsocket : aesdsocket.o
	$(CC) $(CFLAGS) -o aesdsocket aesdsocket.o

aesdsocket.o : aesdsocket.c
	$(CC) $(CFLAGS) -c aesdsocket.c

clean:
	rm -f aesdsocket aesdsocket.o