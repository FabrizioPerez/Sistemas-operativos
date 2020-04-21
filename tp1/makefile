# Directorios
CLIENT_SOURCE=cliente
SERVER_SOURCE=server
BINARY_DIR=bin
#Binarios
CLIENT=client
SERVER=server
SERVER_FS=fileserv
SERVER_AUTH=auth
# Opt de compilacion 
CC=gcc
CFLAGS=-std=gnu11  -Wall -Werror -pedantic -Wextra -Wconversion -O1
all : client server
client : $(CLIENT_SOURCE)/main.c
	mkdir -p $(BINARY_DIR)
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/$(CLIENT).o -c $(CLIENT_SOURCE)/main.c 
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/$(CLIENT)   $(BINARY_DIR)/$(CLIENT).o 
server : $(SERVER_SOURCE)/server.c $(SERVER_SOURCE)/common_structs.h $(SERVER_SOURCE)/fileserv.c $(SERVER_SOURCE)/auth.c
	mkdir -p $(BINARY_DIR)
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/$(SERVER).o -c $(SERVER_SOURCE)/server.c 
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/$(SERVER)   $(BINARY_DIR)/$(SERVER).o 
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/$(SERVER_FS).o -c $(SERVER_SOURCE)/fileserv.c 
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/$(SERVER_FS)   $(BINARY_DIR)/$(SERVER).o 
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/$(SERVER_AUTH).o -c $(SERVER_SOURCE)/auth.c 
	$(CC) $(CFLAGS)  -o $(BINARY_DIR)/$(SERVER_AUTH)   $(BINARY_DIR)/$(SERVER).o 
.PHONY: clean
clean :
	rm  -Rf $(BINARY_DIR)
