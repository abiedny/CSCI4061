CFLAGS = -Wall -g
CC     = gcc $(CFLAGS)

bl_server : bl_server.o server_funcs.o blather.h
	$(CC) -o bl_server bl_server.o server_funcs.o blather.h

bl_server.o : bl_server.c blather.h
	$(CC) -c bl_server.c

bl_client : bl_client.o simpio.o blather.h
	$(CC) -o bl_client bl_client.o simpio.o -lpthread

bl_client.o : bl_client.c blather.h
	$(CC) -c bl_client.c

server_funcs.o : server_funcs.c blather.h
	$(CC) -c server_funcs.c

simpio.o : simpio.c blather.h
	$(CC) -c simpio.c

clean :
	rm -rf *.o
	rm -rf *.fifo
	rm -f bl_client
	rm -f bl_server

include test_Makefile