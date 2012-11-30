CC     := gcc
CFLAGS := 
TARGET := tunnel
OBJS   := tunnel.o network.o

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	rm -f tunnel
	rm -f *.o
