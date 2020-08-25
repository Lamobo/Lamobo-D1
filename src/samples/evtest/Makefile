CC=arm-none-linux-gnueabi-gcc
CFLAGS = -Wall -g
ARCH=arm

all: evtest

gpio: evtest.c
	$(CC) $(CFLAGS) -I../../kernel/include/ \
					-o $@ $^
					
					
					

.PHONY: clean

clean:
	-rm -f evtest $(OBJS)
