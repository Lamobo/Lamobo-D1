/**
 * \file 
	\brief Poll serial data
	
	poll serial data from ttySAK1 port /analyzing / execute scripts
	\author amartol
	\version 1.0
	\date Feb 2018
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
#include <sys/socket.h>
#include <netinet/in.h>
#include "IPCameraCommand.h"

#define REC_PATH		/*"/mnt/akipcserver"*/ "/usr/bin/akipcserver"  ///< Path to recorder
#define REC_NAME		"akipcserver"								 ///< Recorder name
#define REC_PARAM		"-s"										 ///< Recorder send/receive signals
#define REC_PARAM_RTSP	"-sr"										 ///< Recorder send/receive signals & rtsp stream
#define TXT_BUF	100													///< Buffer for text messages


//extern const char * const sys_siglist[];

serial_t* tty;												///< Pointer to serial structure
//char dev[]={"/dev/ttyUSB0"};
char dev[]={"/dev/ttySAK1"}; 								///< Name of serial device
int baud = 115200;											///< Default baudrate
uint8_t rxdata[BUFF_SIZE];									///< Buffer for received data
pid_t pid = -1;												///< Pid child process
volatile sig_atomic_t sig = S_IDLE;							///< Signal flag
siginfo_t s_inf;											///< Structure for handle info of received signal 
int sock;													///<Socket descriptor
struct sockaddr_in addr;									///<Socket address struct
int conn = -1;												///<If connected to socket conn = 0
int flag_translate = 0;								///<If rtsp stream enabled = true
/**
 * @brief Main loop
 * \retval 0 if ok, else return 1
 * 
 * 
 * 
*/
int main(void) {
//
atexit(tpoll_exit);

//initialize signals
if (sig_init() < 0) {
	exit(EXIT_FAILURE);
}
//create serial device connection
tty = serial_create();
if(serial_connect(tty, dev, baud) < 0){
	perror("serial connect");
	exit(EXIT_FAILURE);
}
if (access("/dev/mmcblk0", R_OK) == 0) {
	send_response(CMD_REC_READY);
	fprintf(stdout,"---REC_READY send to MCU\n");
}
	else {
	fprintf(stdout, "---no SD card! CMD_REC_NOSDCARD send\n");
	send_response (CMD_REC_NOSDCARD);
	}


for(;;){
	usleep(100 * 1000); //sleep 100ms
	int cnt;
	if(serial_available(tty)) {
		if((cnt = serial_data_processing(tty)) > 0) {
			//processing cmd_code
			cmd_code_processing (rxdata); 
		} // end if processing
		else {								//error processing rxdata
			fprintf(stdout,"clear rx buffer\n");
			serial_clear(tty);
		}	
	} //end serial available

	
	if(sig != S_IDLE)	{						 //signal received?
		incoming_signal_processing(sig, flag_translate); 
		sig = S_IDLE;
	}
} //end for

	
fprintf(stdout,"exit from main loop\n");
exit(EXIT_SUCCESS);
} 

/**
 * @brief Signal handler
 * @param signal - received signal
 * @param *s_inf - pointer to struct with information about signal
 * @param *ucontext - pointer to  ucontext_t struct contains signal context infos
 */
void sig_hdl(int signal, siginfo_t* s_inf, void* ucontext)
{
	char sigmsg[TXT_BUF];
	switch (signal)
	{
		case SIGUSR1:
			if(s_inf->si_code == SI_QUEUE) {
			switch(s_inf->si_int) {
				case 0:
					sig = S_RECORDER_STOP;
				break;
							
				case 1: 
					sig = S_RECORDER_RUN;
				break;
				
				case 2:
					sig = S_CAMERA_ERROR;
				break;	
			
				default:
				break;
			}
		}
		break;
		
		case SIGCHLD:
			if(!s_inf->si_errno) {
				if	(s_inf->si_code == CLD_EXITED) //if child exit 
					snprintf(sigmsg, TXT_BUF, "Exit with code = %d\n", s_inf->si_status);
				else {
					snprintf(sigmsg, TXT_BUF, "Child exited with signal %d (%s)\n", s_inf->si_code, 
					(s_inf->si_code == CLD_KILLED) ? "CLD_KILLED":
					(s_inf->si_code == CLD_DUMPED) ? "CLD_DUMPED": "other");
				}
			}
			else {
				snprintf(sigmsg,TXT_BUF, "Child exited error: %d, %s\n", s_inf->si_errno, strerror(s_inf->si_errno));
			}
			write(1, sigmsg, strlen(sigmsg)+1);
			sig = S_CHILD_EXIT;
		break;
		
		case SIGINT:
			sig = S_KEY_INT;
		break;
				
		default:
		break;
	}
}

/**
 * @brief Processing & analyze received bytes
 * @param *s - pointer to serial structure.
 * @return received bytes
 */
static int serial_data_processing (serial_t* s)
{
	int i;
	for(i = 0; i < BUFF_SIZE && serial_available(tty); i++){
			rxdata[i] = serial_get(s);
	}
	//fprintf(stdout,"%c %c bytes cnt=%d\n", rxdata[0], rxdata[i-1], i);
	
	//analyze received data
	if((rxdata[0] != STARTMSG) || (rxdata[i-1] != STOPMSG)) {
		fprintf(stdout,"incorrect packet data: first byte 0x%X, last byte 0x%X\n", rxdata[0], rxdata[i-1]);
		//report to host
		return -1;
	}
	
	//analyze crc
	uint16_t crc, crc_rx;
	//fprintf(stdout,"%d %d \n", rxdata[1], rxdata[1]-2);
	crc = gencrc(rxdata+1, rxdata[1] - 4); //calculate crc other than start/stop symbols & received crc
	
	crc_rx = rxdata[i-3] << 8 | rxdata[i-2];
/*	fprintf(stdout,"RECEIVED crc=0x%X\n", crc_rx);
	fprintf(stdout,"CALCULATED crc=0x%X\n",crc); */
	if(crc != crc_rx) {
		fprintf(stdout,"incorrect packet crc: received 0x%X, calculated 0x%X\n", crc_rx, crc);
		return -2;
	}
//fprintf(stdout,"%c %c i=%d\n", rxdata[0], rxdata[i-1], i);
return (i-1);
}

/**
 * @brief Processing & analyze received command
 * 
 * @param *data - pointer to rxdata buffer.
 * */
static void cmd_code_processing (uint8_t* data) 
{
	bool B_mode_wifi = true;
	struct tm 		str_time;
	struct timeval 	now;
	struct sigaction act_chd;
	char param[6];
	uint8_t cmd = *(data+2);
	
	fprintf(stdout,"command %c\n", *(data+2));
	switch (cmd) {
	int i;
	
	case CMD_GET_STATUS:
	if(pid == -1) { 			//if does not have child
		fprintf(stdout,"---%s: recorder ready\n", __func__);
		send_response(CMD_REC_READY);
	}
	else {			//if record run
		if(flag_translate) {
			 send_response(CMD_START_TRANSL);
			 fprintf(stdout,"---%s: status \"translate\" send\n", __func__);
			 }
			 
			 else {
				 fprintf(stdout,"---%s: status \"record\" send\n", __func__);
				 send_response(CMD_START_RECORD);
			}
		
	}
	break;

	case CMD_START_TRANSL:
	
	case CMD_START_RECORD:
			
			if(pid == -1) {			//if does not have child
				if (access("/dev/mmcblk0", R_OK) == 0) {
					pid = fork();								//create new process
					if (pid != 0) {												//parent proc
						fprintf(stdout,"child pid = %d\n", pid);
						if(cmd == CMD_START_TRANSL)	{
							flag_translate = 1;
							fprintf(stdout,"%s: start translate\n", __func__);
						}	
						else {
							flag_translate = 0;
							fprintf(stdout,"%s: start record\n", __func__);
						}			
					}
					else if (!pid) { 										//child proc: close all file desc & ignore SIGINT
						memset (&act_chd, 0, sizeof(act_chd));
						act_chd.sa_handler = SIG_IGN;
						if (sigaction(SIGINT, &act_chd, NULL) < 0) 	//block SIGINT on child
							perror ("failed sigaction block SIGINT");
						serial_close(tty);
						serial_destroy(tty);
						if (cmd == CMD_START_RECORD) {
							strcpy(param, REC_PARAM);
							
						}
						else if (cmd == CMD_START_TRANSL) {
							strcpy(param, REC_PARAM_RTSP);
						}
						
						if(execl(REC_PATH, REC_NAME, param, NULL) < 0) { //execute recorder on child proc
							perror("child exec err");
							exit(EXIT_FAILURE);
						}
					}
					else if (pid < 0) {
						perror ("fork process failed");
						send_response (CMD_REC_ERROR);
					}
				}
				else {
					fprintf(stdout,"no SD card!\n");
					send_response (CMD_REC_NOSDCARD);
				}
			}
	
	break;
	
	case CMD_STOP_RECORD:
		if(pid != -1) {
			fprintf(stdout,"%s: stop record\n", __func__);
			kill(pid, SIGTERM);
		}
	break;
	
	case CMD_GET_PHOTO:
		//get foto
		fprintf(stdout,"get_photo\n");
		
		if( system("pgrep akipcserver") == 0)  {
			if(conn != 0) {
				//create socket to send cmd to akipcserver
			sock = socket(AF_INET, SOCK_STREAM, 0);
			if(sock < 0)
			{
				perror("socket");
				exit(1);
			}
			
			addr.sin_family = AF_INET;
			addr.sin_port = htons(TCPLISTENPORT); 
			addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
				conn = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
					if(conn < 0) {
						perror("connect");
						break;
					}
			}
			send_to_socket(sock);
		}
		else fprintf(stdout,"No record process found!\n");
		
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
			//fprintf(stdout,"%s\n", asctime(&str_time));
			now.tv_usec = 0;
			now.tv_sec = mktime(&str_time);
			if(settimeofday(&now, NULL) != 0) {
				perror("settimeofday() failed\n");
			}
			else {
				fprintf(stdout,"---%s: Adjust time success, status \"CMD_REC_READY\" send\n", __func__);
				send_response(CMD_REC_READY);
			}
		}
		else fprintf(stdout,"packet size for adjust time incorrect.\n");
	break;
	
	case CMD_USB_HOST_MODE:
		if(B_mode_wifi == false) {
			system("/etc/init.d/udisk.sh stop");
			B_mode_wifi = true;
		}
	break;
	
	case CMD_USB_DEVICE_MODE:
		if(B_mode_wifi == true) {
			system("/etc/init.d/udisk.sh start");
			B_mode_wifi = false;
		}
	break;			
	
	default:
	break;
	} //end switch
	
}

/**
 * Send command from the socket to akipcserver 
 * @param sock - socket to send
 * 
*/
static int send_to_socket ( int sock )
{
	
	SYSTEM_HEADER sysHeader;
	CON_SYSTEM_TAG(SYSTEM_TAG, &sysHeader.nSystemTag);
	sysHeader.nCommandCnt = 1;
	sysHeader.nLen = sizeof(SYSTEM_HEADER) * sysHeader.nCommandCnt;
	
	COMMAND_HEADER cmdHeader;
	cmdHeader.CommandType = COMM_TYPE_OPEN_SERVICE;
	cmdHeader.subCommandType = OPEN_COMM_TAKE_PIC;
	cmdHeader.nLen = sizeof(COMMAND_HEADER);
		
	unsigned char buf[TXT_BUF];
	int nSend = 0;
	memset(buf, 0x0, TXT_BUF);
	
	memcpy(buf, &sysHeader, sizeof(sysHeader));
	nSend += sizeof(sysHeader);

	memcpy(buf+nSend, &cmdHeader, sizeof(cmdHeader));
	nSend += sizeof(cmdHeader);

	int ret;
	unsigned char* pbuf = buf;
	while(nSend > 0)
	{
		ret = send(sock, pbuf, nSend, 0);
		if(ret < 0)
		{
			printf("send error\n");
			return -1;
		}
			
		nSend -= ret;
		pbuf += ret;
	}
	return 0;
}


/**
 * @brief Generate  CRC-16/XMODEM
 * 
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
 * @brief Send responce to MCU
 * @param cmd - command to transmit.
 * @return quantity of transmitted bytes 
 */
 static int send_response (uint8_t cmd) 
{
	uint8_t txbuf[32];
	uint16_t crc;
	uint8_t i = 0;
	txbuf[i]	= STARTMSG;
	//----------------------//
	txbuf[++i]	= 0x06; //qty tx bytes
	txbuf[++i] 	= cmd; 
	crc = gencrc(txbuf+1, (size_t) txbuf[1] - 4);
	txbuf[++i]	= crc >> 8; //hi
	txbuf[++i]	= (uint8_t)crc; //low
	//---------------------//
	txbuf[++i] 	= STOPMSG;
	//fprintf(stdout,"response crc = 0x%04X\n", crc);
	serial_clear(tty);
	int j = 0;
	for(j = 0; j <= i; j++)
		serial_put(tty, txbuf[j]);
return (j-1);
}

/**
 * @brief Signals initialization
 * @retval 0 if init success,
 * @retval < 0  if error signal init
 */
static int sig_init (void) 
{
	//signals registration
	int ret = 0;
	struct sigaction act;
	memset (&act, 0, sizeof(act));
	act.sa_sigaction = &sig_hdl;
	//act.sa_handler = &sig_hdl;
	act.sa_flags = SA_SIGINFO | SA_NOCLDWAIT;
	sigset_t   set; 
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGCHLD);
	sigaddset(&set, SIGINT);
	act.sa_mask = set;	//blocked sig
	
	if (sigaction(SIGUSR1, &act, NULL) < 0) {
		perror ("sigaction init USR1");
		ret --;
	}
	if (sigaction(SIGCHLD, &act, NULL) < 0) {
		perror ("sigaction init SIGCHLD");
		ret --;
	}
	if (sigaction(SIGINT, &act, NULL) < 0) {
		perror ("sigaction init SIGINT");
		ret --;
	}
	return ret;
}

/**
 * @brief Signals processing
 * @param signal - value to processing 
 */
static void incoming_signal_processing (sig_atomic_t signal, int flag) 
{
	
	//int state_val = 0;
	switch (signal){
	case S_CHILD_EXIT:
		/*wait(&state_val);
		if (WIFEXITED(state_val))
			fprintf("Child exited with code %d\n", WEXITSTATUS(state_val));
		else if (WIFSIGNALED(state_val))
			fprintf("Child exited by %s signal\n", sys_siglist[WTERMSIG(state_val)]);
		else printf("Child terminated abnormally\n");*/
		//fprintf("%s",sigmsg);
		pid = -1;					//child process exited
		conn = -1;					//connection closed
	break;
	case S_RECORDER_RUN:
	{
	if(flag){
		fprintf(stdout,"---Translate started, send response to MCU\n");
		send_response(CMD_START_TRANSL); 
	}
	else {
		fprintf(stdout,"---Record started, send response to MCU\n");
		send_response(CMD_START_RECORD);
	}
	}
	break;
	
	case S_RECORDER_STOP:
		fprintf(stdout,"---Record stop, send to MCU\n");
		send_response (CMD_STOP_RECORD);
	break;
	
	case S_CAMERA_ERROR:
		fprintf(stdout,"---akipcserver: CAMERA ERROR!\n");
		send_response (CMD_REC_CAM_ERROR);
	break;
	
	case S_KEY_INT:
		fprintf(stdout,"%s: signal SIGINT received\n", __func__);
		exit(EXIT_SUCCESS);
	break;
	
	default:
	break;
	}
	
}

/**
 * @brief Exit from the func
 * 
 */
static void tpoll_exit(void)
{
	fprintf(stdout,"--%s--\n", __func__);
	if(pid != -1){		//if child process work, kill his
		kill(pid, SIGTERM);
		wait(NULL);
	}
	if(sock)
		close(sock);
	serial_close(tty);
	serial_destroy(tty);
	
}

