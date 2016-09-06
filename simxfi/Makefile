CC=g++
CFLAGS=-W -Wall -ansi -pedantic
THREADS=-lpthread
EXEC1=server
EXEC2=client

all: $(EXEC1) $(EXEC2)

server: server.o equipement.o files.o utile.o
	$(CC) -o $@ $^ $(THREADS)

client: client.o files.o utile.o
	$(CC) -o $@ $^ $(THREADS)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o $(EXEC1) $(EXEC2)
