#https://www.gnu.org/software/make/manual/html_node/Simple-Makefile.html

CC = $(CROSS_COMPILE)gcc

default: writer.o
	cc -o writer writer.o

writer.o : writer.c
	cc -c writer.c

clean:
	rm -f writer writer.o