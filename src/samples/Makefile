CC=arm-none-linux-gnueabi-gcc
CFLAGS = -Wall -g

all: gpio-led i2c-test

gpio-led:	gpio-led.c
	$(CC) $(CFLAGS) -o $@ $^

i2c-test:	i2c-test.c
	$(CC) $(CFLAGS) -o $@ $^


.PHONY : clean

clean:
	-rm -f gpio-led i2c-test $(OBJS)
	

