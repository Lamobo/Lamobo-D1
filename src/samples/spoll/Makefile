CC=arm-none-linux-gnueabi-gcc
CFLAGS = -Wall -g
ARCH=arm

all: spoll

spoll: spoll.c
	$(CC) $(CFLAGS) -I../../kernel/include/ \
					-o $@ $^
					
					
					

.PHONY: clean

clean:
	-rm -f spoll $(OBJS)
