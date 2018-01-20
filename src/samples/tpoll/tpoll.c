/*
 * poll serial data from ttySAK1 port /analyzing / execute scripts
 * 
 * @by amartol
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include "uart.h"
#include "tpoll.h"

//int err;
//ssize_t length;
serial_t* tty;
//char dev[]={"/dev/ttyUSB0"};
char dev[]={"/dev/ttySAK1"}; //for production
int baud = 115200;
char rxdata[BUFF_SIZE];


int main(void) {
bool b_mode_wifi = true;
struct tm 		str_time;
struct timeval 	now;


tty = serial_create();
if(serial_connect(tty, dev, baud) < 0){
	perror("serial connect");
	tpoll_exit(-1);
}


for(;;){
	sleep(1);
	int cnt;
	if(serial_available(tty)) {
		if((cnt = rxdata_processing(tty)) > 0) {
			//processing cmd_code
			//printf("command %c\n", rxdata[2]);
			switch (rxdata[2]) {
			int i;
			case CMD_BEGIN_RECORD:
			break;
			
			case CMD_END_RECORD:
			break;
	
			case CMD_CLK_ADJUST:
				if (cnt == 11){
				i = 3;
				str_time.tm_year = 	(int) rxdata[i];
				str_time.tm_mon = 	(int) rxdata[++i];
				str_time.tm_mday = 	(int) rxdata[++i];
				str_time.tm_hour = 	(int) rxdata[++i];
				str_time.tm_min = 	(int) rxdata[++i];
				str_time.tm_sec =	(int) rxdata[++i];
				str_time.tm_isdst = -1;
				//printf("%s\n", asctime(&str_time));
				now.tv_usec = 0;
				now.tv_sec = mktime(&str_time);
				
					if(settimeofday(&now, NULL) == 0) {
						printf("settimeofday() successful.\n");
					}
					else {
						printf("settimeofday() failed, errno = %d\n",errno);
						//exit(EXIT_FAILURE);
					}
				}
				else printf("packet size for time incorrect.\n");
			break;
			
			case CMD_WIFI_MODE:
				if(b_mode_wifi == false) {
					system("/etc/init.d/udisk.sh stop");
					b_mode_wifi = true;
				}
			break;
			
			case CMD_DEVICE_MODE:
				if(b_mode_wifi == true) {
					system("/etc/init.d/udisk.sh stop");
					b_mode_wifi = false;
				}
			break;			
			
			default:
			break;
			} //end switch
		} // end if processing
		else {								//error processing rxdata
			printf("clear rx buffer\n");
			//serial_clear(tty);
		}	
	} //end serial available

} //end for
	
	

	
	
printf("exit from main loop\n");
serial_close(tty);
serial_destroy(tty);
exit(0);
} 

/**
 * Processing & analyze received bytes
 * 
 * 
 * @param *s - pointer to serial structure.
 * @return received bytes
 */
static int rxdata_processing (serial_t* s)
{
	int i;
	for(i = 0; i < BUFF_SIZE && serial_available(tty); i++){
			rxdata[i] = serial_get(s);
	}
	//printf("%c %c cnt=%d\n", rxdata[0], rxdata[i-1], i-1);
	
	//analyze received data
	if((rxdata[0] != STARTMSG) || (rxdata[i-1] != STOPMSG)) {
		printf("incorrect packet data: first byte 0x%X, last byte 0x%X\n", rxdata[0], rxdata[i-1]);
		//report to host
		return -1;
	}
	
	//analyze crc
	u16 crc, crc_rx;
	crc = gencrc((u8*)rxdata+1, (size_t) rxdata[1] - 2); //calculate other than start/stop symbols
	crc_rx = (u8)rxdata[i-3] << 8 | (u8)rxdata[i-2];
	//printf("RECEIVED crc=0x%X\n", crc_rx);
	//printf("CALCULATED crc=0x%X\n",crc);
	if(crc != crc_rx) {
		printf("incorrect packet crc!\n");
		return -2;
	}
//printf("%c %c i=%d\n", rxdata[0], rxdata[i-1], i);
return (i-1);
}


/**
 * Generate  CRC-16/XMODEM
 * poly=0x1021, initial value=0x0000
 * @param *bfr - pointer to input data buffer.
 * @param len - bytes qty 
 * @return calculated value
 */
static u16 gencrc(u8 *bfr, size_t len)
{
u16 crc = 0x0;
    while(len--)
        crc = (crc << 8) ^ crctbl[(crc >> 8)^*bfr++];
    return(crc);
}

/**
 * Exit from the func
 * @param sig - signal to send parent process
 */
static void tpoll_exit(int sig)
{
	serial_close(tty);
	serial_destroy(tty);
	exit(sig);
}

