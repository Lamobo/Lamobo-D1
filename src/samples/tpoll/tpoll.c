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
#include <signal.h>
#include <sys/wait.h>
#include "uart.h"
#include "tpoll.h"

#define REC_PATH	"/mnt/akipcserver"//"/usr/bin/akipcserver"
#define REC_NAME	"akipcserver"
//int err;
extern const char * const sys_siglist[];
serial_t* tty;
//char dev[]={"/dev/ttyUSB0"};
char dev[]={"/dev/ttySAK1"}; //for production
int baud = 115200;
uint8_t rxdata[BUFF_SIZE];
//u8 crct[]={"123"};

pid_t pid = -1;
volatile sig_atomic_t sig = IDLE;

int main(void) {
//
atexit(tpoll_exit);
if (sig_init() < 0) {
	exit(EXIT_FAILURE);
}
//create serial device connection
tty = serial_create();
if(serial_connect(tty, dev, baud) < 0){
	perror("serial connect");
	exit(EXIT_FAILURE);
}

for(;;){
	usleep(200000);
	int cnt;
	if(serial_available(tty)) {
		if((cnt = rxdata_processing(tty)) > 0) {
			//processing cmd_code
			cmd_code_processing (rxdata); 
		} // end if processing
		else {								//error processing rxdata
			printf("clear rx buffer\n");
			serial_clear(tty);
		}	
	} //end serial available

	//signal received?
	if(sig != IDLE)	{
		signal_processing(sig); 
		sig = IDLE;
	}
} //end for

	
printf("exit from main loop\n");
exit(EXIT_SUCCESS);
} 

/**
 * signal handler
 * @param signal - received signal
 */
void sig_hdl(int signal)
{
	switch (signal)
	{
		case SIGUSR1:
			sig = MSG_USR1;
		break;
		
		case SIGCHLD:
			sig = CHILD_EXIT;
		break;
		
		case SIGINT:
			sig = KEY_INT;
		break;
				
		default:
		break;
	}
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
	//printf("%c %c bytes cnt=%d\n", rxdata[0], rxdata[i-1], i);
	
	//analyze received data
	if((rxdata[0] != STARTMSG) || (rxdata[i-1] != STOPMSG)) {
		printf("incorrect packet data: first byte 0x%X, last byte 0x%X\n", rxdata[0], rxdata[i-1]);
		//report to host
		return -1;
	}
	
	//analyze crc
	uint16_t crc, crc_rx;
	//printf("%d %d \n", rxdata[1], rxdata[1]-2);
	crc = gencrc(rxdata+1, rxdata[1] - 4); //calculate crc other than start/stop symbols & received crc
	
	crc_rx = rxdata[i-3] << 8 | rxdata[i-2];
	//printf("RECEIVED crc=0x%X\n", crc_rx);
	//printf("CALCULATED crc=0x%X\n",crc);
	if(crc != crc_rx) {
		printf("incorrect packet crc: received 0x%X, calculated 0x%X\n", crc_rx, crc);
		return -2;
	}
//printf("%c %c i=%d\n", rxdata[0], rxdata[i-1], i);
return (i-1);
}

/**
 * Processing & analyze received command
 * 
 * 
 * @param *data - pointer to rxdata buffer.
 *
 */
static void cmd_code_processing (uint8_t* data) 
{
	bool B_mode_wifi = true;
	struct tm 		str_time;
	struct timeval 	now;
	struct sigaction act_chd;
	
	printf("command %c\n", *(data+2));
	switch (*(data+2)) {
	int i;
	case CMD_START_RECORD:
			printf("%s: start record\n", __func__);
			if(pid == -1) {
				if (access("/dev/mmcblk0", R_OK) == 0) {
					pid = fork();	//create new process
					if (pid != 0) {	//parent proc
						printf("child pid = %d\n", pid);					
					}
					else if (!pid) { //child proc
						memset (&act_chd, 0, sizeof(act_chd));
						act_chd.sa_handler = SIG_IGN;
						if (sigaction(SIGINT, &act_chd, NULL) < 0) 
							perror ("failed sigaction block SIGINT");
						serial_close(tty);
						serial_destroy(tty);
							
						if(execl(REC_PATH, REC_NAME, NULL) < 0) {
							perror("child exec err");
							exit(EXIT_FAILURE);
						}
					}
					else if (pid < 0) {
						perror ("fork process failed");
						send_responce (REC_ERROR);
					}
				}
				else {
					printf("no SD card!\n");
					send_responce (REC_NO_SDCARD);
				}
			}
	
	break;
	
	case CMD_STOP_RECORD:
		if(pid != -1) {
			printf("%s: stop record\n", __func__);
			kill(pid, SIGTERM);
		}
	break;
	
	case CMD_CLK_ADJUST:
		if (*(data+1) == 0x0C){ 		//if packet size less or more: error
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
			if(settimeofday(&now, NULL) != 0) {
				perror("settimeofday() failed\n");
			}
		}
		else printf("packet size for adjust time incorrect.\n");
	break;
	
	case CMD_USB_HOST_MODE:
		if(B_mode_wifi == false) {
			system("/etc/init.d/udisk.sh stop");
			B_mode_wifi = true;
		}
	break;
	
	case CMD_USB_DEVICE_MODE:
		if(B_mode_wifi == true) {
			system("/etc/init.d/udisk.sh stop");
			B_mode_wifi = false;
		}
	break;			
	
	default:
	break;
	} //end switch
	
}

/**
 * Generate  CRC-16/XMODEM
 * poly=0x1021, initial value=0x0000
 * @param *bfr - pointer to input data buffer.
 * @param len - bytes qty 
 * @return calculated value
 */
static uint16_t gencrc(uint8_t *bfr, size_t len)
{
uint16_t crc = 0x0;
    while(len--)
        crc = (crc << 8) ^ crctbl[(crc >> 8)^*bfr++];
    return crc;
}


/**
 * Send responce to MCU
 * @param cmd - command to transmit.
 * @return transmitted bytes 
 */
 
static int send_responce (uint8_t cmd) 
{
	uint8_t txbuf[32];
	uint16_t crc;
	uint8_t i = 0;
	txbuf[i]	= STARTMSG;
	//----------------------//
	txbuf[++i]	= 0x06; //qty tx bytes
	txbuf[++i] 	= cmd; 
	crc = gencrc(txbuf+1, (size_t) txbuf[1] - 2);
	txbuf[++i]	= crc >> 8; //hi
	txbuf[++i]	= (uint8_t)crc; //low
	//---------------------//
	txbuf[++i] 	= STOPMSG;
	int j = 0;
	for(j = 0; j <= i; j++)
		serial_put(tty, txbuf[j]);
return (j-1);
}
/**
 * Signals initialization
 * @return sigexit - signal to send parent process
 */
static int sig_init (void) 
{
	//signals registration
	int ret = 0;
	struct sigaction act;
	memset (&act, 0, sizeof(act));
	act.sa_handler = &sig_hdl;
	sigset_t   set; 
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGCHLD);
	sigaddset(&set, SIGINT);
	act.sa_mask = set;	//blocked sig
	
	if (sigaction(SIGUSR1, &act, NULL) < 0) {
		perror ("sigaction init USR1");
		ret += -1;
	}
	if (sigaction(SIGCHLD, &act, NULL) < 0) {
		perror ("sigaction init SIGCHLD");
		ret += -1;
	}
	if (sigaction(SIGINT, &act, NULL) < 0) {
		perror ("sigaction init SIGINT");
		ret += -1;
	}
	return ret;
}

/**
 * Signals processing
 * @param signal - value to process 
 */
static void signal_processing (sig_atomic_t signal) 
{
	int state_val = 0;
	switch (signal){
	case CHILD_EXIT:
		wait(&state_val);
		if (WIFEXITED(state_val))
			printf("Child exited with code %d\n", WEXITSTATUS(state_val));
		else if (WIFSIGNALED(state_val))
			printf("Child exited by %s signal\n", sys_siglist[WTERMSIG(state_val)]);
		else printf("Child terminated abnormally\n");
		pid = -1;
	break;
	
	case MSG_USR1:
		printf("%s: signal SIGUSR1 received\n", __func__);
	break;
	
	case KEY_INT:
		printf("%s: signal SIGINT received\n", __func__);
		exit(EXIT_SUCCESS);
	break;
	
	default:
	break;
	}
}

/**
 * Exit from the func
 * 
 */
static void tpoll_exit(void)
{
	printf("%s\n", __func__);
	if(pid != -1){		//if child process work, kill him
		kill(pid,SIGTERM);
		wait(NULL);
	}
	serial_close(tty);
	serial_destroy(tty);
	
}

