all: tunnel

CC     := gcc
CFLAGS := -Iinclude

tunnel : tunnel.c
	$(CC) $(CFLAGS) tunnel.c -o tunnel

clean :
	rm -f tunnel
