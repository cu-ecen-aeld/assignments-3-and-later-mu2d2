# Makefile for aesdsocket server application
# This was partially modified with the help of ChatGPT
#Link to chat session: https://chatgpt.com/share/6990b762-3dd8-8008-b70d-03f9fc138aca
CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall -Wextra -Werror
TARGET ?= aesdsocket
LDFLAGS ?= -lpthread -lrt

.PHONY: all clean
all: $(TARGET)

default: aesdsocket

$(TARGET) : aesdsocket.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) aesdsocket.o

aesdsocket.o : aesdsocket.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o