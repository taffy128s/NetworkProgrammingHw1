CC = gcc -Wall
SERVER = server.c
CLIENT = client.c

compile: $(SERVER) $(CLIENT)
	$(CC) -o server $(SERVER)
	$(CC) -o client $(CLIENT)

clean: $(OBJS)
	rm server client

