CC = arm-none-linux-gnueabi-gcc

MC = fha_char_interface.o md5.o httpclient.o ftpclient.o updater.o 

updater: $(MC)
	$(CC) -o $@ $(MC) -lpthread

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $< -lpthread

clean:
	rm -rf *.o updater

