CC = g++
CXXFLAGS = -Wall -std=c++17
LDFLAGS = -lpthread -lsqlite3

all: server

server: server.cpp
	$(CC) $(CXXFLAGS) server.cpp -o server $(LDFLAGS)

clean:
	rm -f server *.o
