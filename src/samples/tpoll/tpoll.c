#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include "uart.h"

int err;
ssize_t length;
serial_t* tty;
char dev[]={"/dev/ttySAK1"};
//char txbuf[]={"Hi!\n"};
int baud = 115200;

void tpoll_exit(int sig)
{
serial_close(tty);
serial_destroy(tty);
exit(sig);
}
int main(void) {
char txbuf[BUFF_SIZE];

tty = serial_create();

if(serial_connect(tty, dev, baud) < 0){
	perror("serial connect");
	tpoll_exit(-1);
}
int rxbytes = 0;
//system("ps H");	

for(;;){
	printf("%c ",serial_blocking_get(tty));
	/*if(serial_available(tty)){
		rxbytes = serial_available(tty);
		int i;
		for(i=0; i < rxbytes; i++){
		txbuf[i] = 	serial_get(tty);
		}
		txbuf[rxbytes]= '\0';
		printf("%s ", txbuf);
		putchar('\n');
	}
	//usleep(500000);
	sleep(1);*/
}
tpoll_exit(0);
} 

