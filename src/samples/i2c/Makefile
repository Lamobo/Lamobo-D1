CC=arm-none-linux-gnueabi-gcc
CFLAGS = -Wall -g

all: i2c-test

gpio-led: gpio-led.c
	$(CC) $(CFLAGS) -I../kernel/include -o $@ $^

i2c-test: i2c-test.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	-rm -f i2c-test $(OBJS)
