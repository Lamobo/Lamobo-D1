CC=arm-none-linux-gnueabi-gcc
CFLAGS = -Wall -g -pthread
ARCH=arm
export PATH := /home/user/videorec/compiler/arm-2009q3/bin:$(PATH)


all: tpoll

tpoll: tpoll.c uart.c
	$(CC) $(CFLAGS) \
					-o $@ $^
					
					
					

.PHONY: clean

clean:
	-rm -f tpoll *.o
