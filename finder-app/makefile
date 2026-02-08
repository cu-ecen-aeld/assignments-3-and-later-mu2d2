#https://www.gnu.org/software/make/manual/html_node/Simple-Makefile.html

CC = $(CROSS_COMPILE)gcc

.PHONY: all
all: default

default: writer.o
	$(CC) -o writer writer.o

writer.o : writer.c
	$(CC) -c writer.c

clean:
	rm -f writer writer.o