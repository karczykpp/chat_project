CC = g++
CFLAGS = -Wall -std=c++17 -lpthread -lsqlite3

all: server

server: server.cpp
	$(CC) $(CFLAGS) -o server server.cpp

clean:
	rm -f server *.o
