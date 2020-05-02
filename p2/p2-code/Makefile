CFLAGS = -Wall -g
CC     = gcc $(CFLAGS)

bl_server : bl_server.o server_funcs.o blather.h
	$(CC) -o bl_server bl_server.o server_funcs.o blather.h

bl_server.o : bl_server.c blather.h
	$(CC) -c bl_server.c

bl_client : bl_client.c simpio.o blather.h
	$(CC) -o bl_client bl_client.c simpio.o

server_funcs.o : server_funcs.c blather.h
	$(CC) -c server_funcs.c

simpio.o : simpio.c blather.h
	$(CC) -c simpio.c