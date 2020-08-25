CC=arm-none-linux-gnueabi-gcc
CFLAGS = -Wall -g
ARCH=arm

all: siso

gpio: siso.c
	$(CC) $(CFLAGS) -I../../kernel/include/ \
					-o $@ $^
					
					
					

.PHONY: clean

clean:
	-rm -f siso $(OBJS)
