CC=arm-none-linux-gnueabi-gcc
CFLAGS = -Wall -g

all: gpio-led

gpio-led: gpio-led.c
	$(CC) $(CFLAGS) -I../../kernel/include -o $@ $^

.PHONY: clean

clean:
	-rm -f gpio-led $(OBJS)
