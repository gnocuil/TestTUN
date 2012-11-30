all: tunnel

tunnel : tunnel.c
	gcc tunnel.c -o tunnel

clean :
	rm tunnel
