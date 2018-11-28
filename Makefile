CC=gcc
AR = ar
ARFLAGS = ru
RANLIB = ranlib
CFLAGS= -g
SRCS= client.c server.c
LIBS = -L./SocketLibrary/

all:: socketlib client server example_thread threadpool_test
	strip client
	strip server
	strip example_thread
	strip threadpool_test

socketlib:
	cd SocketLibrary && make

example_thread: example_thread.o
	$(CC) -o example_thread example_thread.o -lpthread

client: client.o common.o
	$(CC) -o client client.o common.o $(LIBS) -lsock -lpthread

server: server.o common.o threadpool.o 
	g++ -o server server.o common.o threadpool.o -lpthread

threadpool_test: threadpool_test.o threadpool.o
	g++ -o threadpool_test threadpool_test.o threadpool.o -lpthread

client.o: client.c common.h
	$(CC) -o client.o -c client.c

server.o: server.c common.h threadpool.h
	g++ -o server.o -c server.c

common.o: common.c
	$(CC) -o common.o -c common.c

example_thread.o: example_thread.c
	$(CC) -o example_thread.o -c example_thread.c

threadpool.o: threadpool.cpp
	g++ -o threadpool.o -c threadpool.cpp

threadpool_test.o: threadpool_test.c threadpool.h
	g++ -o threadpool_test.o -c threadpool_test.c

clean:
	/bin/rm -f client server example_thread threadpool_test *.o core *~ #*
	cd SocketLibrary && make clean
