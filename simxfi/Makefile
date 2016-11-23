CC=g++
CFLAGS=-W -Wall -ansi -pedantic -O0 -g
THREADS=-lpthread
LIBXML=-I/usr/include/libxml2
EXEC1=server
EXEC2=client
EXEC3=xmlmanager

all: $(EXEC1) $(EXEC2) $(EXEC3)

server: server.o equipement.o files.o utile.o
	$(CC) -o $@ $^ $(THREADS)

client: client.o files.o utile.o
	$(CC) -o $@ $^ $(THREADS)

xmlmanager: xmlmanager.o
	$(CC) -o $@ $^ -lxml2

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS) $(LIBXML)

clean:
	rm -rf *.o $(EXEC1) $(EXEC2) $(EXEC3)
