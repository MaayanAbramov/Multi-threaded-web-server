#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#

OBJS = server.o request.o segel.o client.o log.o queue.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall -DNDEBUG
# CFLAGS = -g -Wall

LIBS = -lpthread

.SUFFIXES: .c .o

server: server.o request.o segel.o log.o queue.o
	$(CC) $(CFLAGS) -o server queue.o server.o request.o segel.o log.o $(LIBS)


all: server client output.cgi
	-mkdir -p public
	-cp output.cgi favicon.ico home.html public


client: client.o segel.o
	$(CC) $(CFLAGS) -o client client.o segel.o

output.cgi: output.c
	$(CC) $(CFLAGS) -o output.cgi output.c

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) server client output.cgi
	-rm -rf public
test: tests/*
	-rm -rf tests/get_request.txt tests/post_request.txt
