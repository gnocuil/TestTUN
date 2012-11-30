all: tunnel

CC     := gcc
CFLAGS := -Iinclude -I.

tunnel : tunnel.c
	$(CC) $(CFLAGS) tunnel.c -o tunnel

clean :
	rm -f tunnel
