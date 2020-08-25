/**
 * \file 
	\brief Poll serial data daemon
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
#include "inisetting.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "IPCameraCommand.h"
#include <sys/mount.h>

#define REC_PATH		/*"/mnt/akipcserver"*/ "/usr/bin/akipcserver"  	///< Path to recorder
#define REC_NAME		"akipcserver"									///< Recorder name
#define REC_PARAM		"-s"											///< Recorder send/receive signals
#define REC_PARAM_RTSP	"-sr"											///< Recorder send/receive signals & rtsp stream
#define REC_PARAM_AUD	"-sa"											///< Recorder send/receive signals & audio record
#define REC_PARAM_RTSP_AUD	"-sra"										///< Recorder send/receive signals & rtsp stream & audio record
#define TXT_BUF	100														///< Buffer for text messages

//#define TPOLL_DBG 												///< Enable debug info

//extern const char * const sys_siglist[];

serial_t* tty;												///< Pointer to serial structure
//char dev[]={"/dev/ttyUSB0"};
char dev[]={"/dev/ttySAK1"};							///< Name of serial device
const int baud = 115200;									///< Default baudrate
uint8_t rxdata[BUFF_SIZE];									///< Buffer for received data
pid_t pid = -1;												///< Pid child recprocess
pid_t pid_ud = -1;											///< Pid child usb disk process
volatile sig_atomic_t sig = S_IDLE;							///< Signal flag
siginfo_t s_inf;											///< Structure for handle info of received signal 
int sock;													///<Socket descriptor
struct sockaddr_in addr;									///<Socket address struct
int conn = -1;												///<If connected to socket conn = 0
bool b_flag_translate = true;								///<If rtsp stream enabled = true
bool b_flag_audio = false;									///<If audio enabled = true

uint8_t g_osd = HIDE;										///<Date string  position on osd from ini file
uint8_t t_osd = RIGHT_UP;									///<Date string  position on osd from MCU
uint8_t g_resolution = VIDEO_MODE_QVGA;						///<Resolution from ini file
uint8_t t_resolution = VIDEO_MODE_VGA;						///<Video resolution from MCU


bool g_time_osd = false;									///<Display time on osd or not from ini file
bool t_time_osd = true;										///<Display time on osd or not from MCU
extern struct setting_info setting;
struct picture_info *picture = &setting.picture;
struct video_info *video = &setting.video; 
/**
 * @brief Main loop
 * \retval 0 if ok, else return 1
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
	#ifdef TPOLL_DBG
	fprintf(stdout,"---REC_READY send to MCU\n");
	#endif
}
	else {
	#ifdef TPOLL_DBG
	fprintf(stdout, "---no SD card! CMD_REC_NOSDCARD send\n");
	#endif
	send_response (CMD_REC_NOSDCARD);
	}
//read camera ini
read_osd_ini();

for(;;){
	usleep(100 * 1000); //sleep 100ms
	int cnt;
	if(serial_available(tty)) {
		if((cnt = serial_data_processing(tty)) > 0) {
			serial_clear(tty);
			//processing cmd_code
			cmd_code_processing (rxdata); 
		} // end if processing
		else {								//error processing rxdata
			#ifdef TPOLL_DBG
			fprintf(stdout,"clear rx buffer\n");
			#endif
			serial_clear(tty);
		}	
	} //end serial available

	
	if(sig != S_IDLE)	{						 //signal received?
		incoming_signal_processing(sig, b_flag_translate); 
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
					snprintf(sigmsg, TXT_BUF, "Child proc exit with code = %d\n", s_inf->si_status);
				else {
					snprintf(sigmsg, TXT_BUF, "Child exited with signal %d (%s)\n", s_inf->si_code, 
					(s_inf->si_code == CLD_KILLED) ? "CLD_KILLED":
					(s_inf->si_code == CLD_DUMPED) ? "CLD_DUMPED": "other");
				}
			}
			else {
				snprintf(sigmsg,TXT_BUF, "Child exited error: %d, %s\n", s_inf->si_errno, strerror(s_inf->si_errno));
			}
			#ifdef TPOLL_DBG
			write(1, sigmsg, strlen(sigmsg)+1);
			#endif
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
	#ifdef TPOLL_DBG
	fprintf(stdout,"--------incoming %s\n", __func__);
	#endif
	for(i = 0; i < BUFF_SIZE && serial_available(tty); i++){
			rxdata[i] = serial_get(s);
	}
	#ifdef TPOLL_DBG
	for(i = 0; i < rxdata[1]; i++){ 
	fprintf(stdout,"rxdata[%d] = %d \n", i, rxdata[i]);
	}
	#endif
	//analyze received data
	if((rxdata[0] != STARTMSG) || (rxdata[i-1] != STOPMSG)) {
		#ifdef TPOLL_DBG
		fprintf(stdout,"incorrect packet data: first byte 0x%X, last byte 0x%X\n", rxdata[0], rxdata[i-1]);
		#endif
		//report to host
		return -1;
	}
	
	//analyze crc
	uint16_t crc, crc_rx;
	//fprintf(stdout,"%d %d \n", rxdata[1], rxdata[1]-2);
	crc = gencrc(rxdata+1, rxdata[1] - 4); //calculate crc other than start/stop symbols & received crc
	
	crc_rx = rxdata[i-3] << 8 | rxdata[i-2];
	#ifdef TPOLL_DBG
	fprintf(stdout,"RECEIVED crc=0x%X\n", crc_rx);
	fprintf(stdout,"CALCULATED crc=0x%X\n",crc); 
	#endif
	if(crc != crc_rx) {
		#ifdef TPOLL_DBG
		fprintf(stdout,"incorrect packet crc: received 0x%X, calculated 0x%X\n", crc_rx, crc);
		#endif
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
	//bool B_mode_wifi = true;
	struct tm 		str_time;
	struct timeval 	now;
	struct sigaction act_chd;
	char param[6];
	uint8_t cmd = *(data+2);
	#ifdef TPOLL_DBG
	fprintf(stdout,"command %c\n", *(data+2));
	#endif
	switch (cmd) {
	int i;
	
	case CMD_GET_STATUS:
	
	if (access("/dev/mmcblk0", R_OK) != 0) {
	#ifdef TPOLL_DBG
	fprintf(stdout, "---no SD card! CMD_REC_NOSDCARD send\n");
	#endif
	send_response (CMD_REC_NOSDCARD);
	break;
	}
	
	if(pid == -1) { 			//if does not have child
		#ifdef TPOLL_DBG
		fprintf(stdout,"---%s: recorder ready\n", __func__);
		#endif
		send_response(CMD_REC_READY);
	}
	else {			//if record run
		if(b_flag_translate) {
			 send_response(CMD_START_TRANSL);
			 #ifdef TPOLL_DBG
			 fprintf(stdout,"---%s: status \"translate\" send\n", __func__);
			 #endif
			 }
			 
			 else {
				 send_response(CMD_START_RECORD);
				 #ifdef TPOLL_DBG
				 fprintf(stdout,"---%s: status \"record\" send\n", __func__);
				 #endif
			}
		
	}
	break;

	case CMD_START_TRANSL:
	
	case CMD_START_RECORD:
			
			if(pid == -1) {			//if does not have child
				if (access("/dev/mmcblk0", R_OK) == 0) {
					pid = fork();								//create new process
					if (pid != 0) {												//parent proc
						#ifdef TPOLL_DBG
						fprintf(stdout,"child pid = %d\n", pid);
						#endif
						if(cmd == CMD_START_TRANSL)	{
							b_flag_translate = true;
							#ifdef TPOLL_DBG
							fprintf(stdout,"%s: start translate\n", __func__);
							#endif
						}	
						else {
							b_flag_translate = false;
							#ifdef TPOLL_DBG
							fprintf(stdout,"%s: start record\n", __func__);
							#endif
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
							if(b_flag_audio) strcpy(param, REC_PARAM_AUD);
							else strcpy(param, REC_PARAM);
							
						}
						else if (cmd == CMD_START_TRANSL) {
							if(b_flag_audio) strcpy(param, REC_PARAM_RTSP_AUD);
							else strcpy(param, REC_PARAM_RTSP);
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
			else {
				if(b_flag_translate)  send_response(CMD_START_TRANSL);	 	 
				else  send_response(CMD_START_RECORD);
			}
	
	break;
	
	case CMD_STOP_RECORD:
		if(pid != -1) {
			#ifdef TPOLL_DBG
			fprintf(stdout,"%s: stop record\n", __func__);
			#endif
			kill(pid, SIGTERM);
		}
		else send_response (CMD_STOP_RECORD);
	break;
	
	case CMD_GET_PHOTO:
		//get foto
		#ifdef TPOLL_DBG
		fprintf(stdout,"get_photo\n");
		#endif
		if( pid != -1)  {
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
			send_response (CMD_GET_PHOTO);
		}
		else fprintf(stdout,"No record process found!\n");
		
	break;
	
	case CMD_CLK_ADJUST:
		if (*(data+1) == 14){ 		//if packet size less or more: error
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
			//take osd position
			t_osd = rxdata[++i];
			t_time_osd = rxdata[++i];
			
			//read camera ini file
			read_osd_ini();
			if( (t_osd != g_osd) || (t_time_osd != g_time_osd) ) {
				if(!write_osd_ini()) {
					g_osd = t_osd;
					g_time_osd = t_time_osd;
				}
				else {
					perror("Adjust osd position error\n");
					break;
				}
				
			}
			#ifdef TPOLL_DBG
			fprintf(stdout,"---%s: Adjust time success, status \"CMD_CLK_ADJUST\" send\n", __func__);
			#endif
			send_response(CMD_CLK_ADJUST);
			
		}
		else fprintf(stdout,"packet size for adjust time incorrect.\n");
	break;
	
	case CMD_RES_ADJUST:
		if (*(data+1) == 8){ 
			i=3;
			//take video resolution
			t_resolution = data[i];
			if(t_resolution != g_resolution) {
				if(!write_video_ini()) {
					g_resolution = t_resolution;
				}
				else {
					perror("Adjust video resolution error\n");
					break;
				}
			}
			i++;
			if(data[i])	b_flag_audio = true; //get audio rec possibility
			else b_flag_audio = false;
			
			#ifdef TPOLL_DBG
			fprintf(stdout,"---%s: Adjust video resolution success.Send CMD_RES_ADJUST\n", __func__);
			#endif
			send_response(CMD_RES_ADJUST);
		}
		else fprintf(stdout,"packet size for adjust video resolution incorrect.\n");
	
	break;
	
	case CMD_DISK_FORMAT: {
		#ifdef TPOLL_DBG
		fprintf(stdout,"---%s: Format sdcard\n", __func__);
		#endif
		pid_ud = fork();
		if (pid_ud != 0) {												//parent proc
				#ifdef TPOLL_DBG
				fprintf(stdout,"wait child pid = %d\n", pid_ud);
				#endif
				waitpid(pid_ud, NULL, 0);
				pid_ud = -1;
				#ifdef TPOLL_DBG
				fprintf(stdout,"---%s: format success, status \"CMD_DISK_FORMAT\" send\n", __func__);
				#endif
				send_response(CMD_DISK_FORMAT);
		}
		else if (!pid_ud) { 										//child proc: close all file desc & ignore SIGINT
				memset (&act_chd, 0, sizeof(act_chd));
				act_chd.sa_handler = SIG_IGN;
				if (sigaction(SIGINT, &act_chd, NULL) < 0) 	//block SIGINT on child
					perror ("failed sigaction block SIGINT");
				serial_close(tty);
				serial_destroy(tty);
				if (execl("/bin/busybox", "mkfs.vfat", "/dev/mmcblk0p1", NULL) < 0)
					perror("execl failed");
		}
	}
	break;
	
	case CMD_USB_HOST_MODE:
			#ifdef TPOLL_DBG
			fprintf(stdout,"---%s: Stop Udisk\n", __func__);
			#endif
			pid_ud = fork();
			if (pid_ud != 0) {												//parent proc
				#ifdef TPOLL_DBG
				fprintf(stdout,"wait child usbpid = %d\n", pid_ud);
				#endif
				waitpid(pid_ud, NULL, 0);
				pid_ud = -1;
				#ifdef TPOLL_DBG
				fprintf(stdout,"---%s: Change USB mode to HOST, status \"CMD_USB_HOST_MODE\" send\n", __func__);
				#endif
				send_response(CMD_USB_HOST_MODE);
			}
			else if (!pid_ud) { 										//child proc: close all file desc & ignore SIGINT
				memset (&act_chd, 0, sizeof(act_chd));
				act_chd.sa_handler = SIG_IGN;
				if (sigaction(SIGINT, &act_chd, NULL) < 0) 	//block SIGINT on child
					perror ("failed sigaction block SIGINT");
				serial_close(tty);
				serial_destroy(tty);
				if (execl("/etc/init.d/udisk.sh", "udisk.sh", "stop", NULL) < 0)
					perror("execl failed");
				}
	break;
	
	case CMD_USB_DEVICE_MODE:
			#ifdef TPOLL_DBG
			fprintf(stdout,"---%s: Start Udisk\n", __func__);
			#endif
			pid_ud = fork();
			if (pid_ud != 0) {												//parent proc
				#ifdef TPOLL_DBG
				fprintf(stdout,"wait child usbpid = %d\n", pid_ud);
				#endif
				waitpid(pid_ud, NULL, 0);
				pid_ud = -1;
				#ifdef TPOLL_DBG
				fprintf(stdout,"---%s: Change USB mode to DEVICE, status \"CMD_USB_DEVICE_MODE\" send\n", __func__);
				#endif
				send_response(CMD_USB_DEVICE_MODE);
			}
			else if (!pid_ud) { 										//child proc: close all file desc & ignore SIGINT
				memset (&act_chd, 0, sizeof(act_chd));
				act_chd.sa_handler = SIG_IGN;
				if (sigaction(SIGINT, &act_chd, NULL) < 0) 	//block SIGINT on child
					perror ("failed sigaction block SIGINT");
				serial_close(tty);
				serial_destroy(tty);
				if(execl("/etc/init.d/udisk.sh", "udisk.sh", "start", NULL) < 0)
					perror("execl failed");
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
		if (pid != -1) {
		pid = -1;					//child process exited
		conn = -1;					//connection closed
		}
	break;
	case S_RECORDER_RUN:
	{
	if(flag){
		#ifdef TPOLL_DBG
		fprintf(stdout,"---Translate started, send CMD_START_TRANSL to MCU\n");
		#endif
		send_response(CMD_START_TRANSL); 
	}
	else {
		#ifdef TPOLL_DBG
		fprintf(stdout,"---Record started, send CMD_START_RECORD to MCU\n");
		#endif
		send_response(CMD_START_RECORD);
	}
	}
	break;
	
	case S_RECORDER_STOP:
		#ifdef TPOLL_DBG
		fprintf(stdout,"---Record stop, send CMD_STOP_RECORD to MCU\n");
		#endif
		send_response (CMD_STOP_RECORD);
	break;
	
	case S_CAMERA_ERROR:
		#ifdef TPOLL_DBG
		fprintf(stdout,"---akipcserver: CAMERA ERROR!\n");
		#endif
		send_response (CMD_REC_CAM_ERROR);
	break;
	
	case S_KEY_INT:
		#ifdef TPOLL_DBG
		fprintf(stdout,"%s: signal SIGINT received\n", __func__);
		#endif
		exit(EXIT_SUCCESS);
	break;
	
	default:
	break;
	}
	
}

/**
 * @brief Read camera ini file
 *
 */
static int read_osd_ini(void)
{
	IniSetting_init();
	//get pic info
	picture = IniSetting_GetPictureInfo();
	video = IniSetting_GetVideoResolution();
	/*
	fprintf(stdout,"-osd name from ini file: %s\n", picture->osd_name);
	fprintf(stdout,"-osd place from ini file: %s\n", picture->osd_place);
	fprintf(stdout,"-osd time from ini file: %s\n", picture->osd_time);
	*/
	if(strlen(picture->osd_name) == 0)
	{
		g_osd = HIDE;
	}
	else {
		if( !strcmp(picture->osd_place, "right_up"))
		{
			g_osd = RIGHT_UP;
		}
		else if(!strcmp(picture->osd_place, "right_down"))
		{
			g_osd = RIGHT_DOWN;
		}
		else if(!strcmp(picture->osd_place, "left_up"))
		{
			g_osd = LEFT_UP;
		}
		else if(!strcmp(picture->osd_place, "left_down"))
		{
			g_osd = LEFT_DOWN;
		}
		else 
		{
			g_osd = HIDE;
		}
	}

	if(!strcmp(picture->osd_time, "show"))
	{
		g_time_osd = true;
	}
	else if (!strcmp(picture->osd_time, "hide"))
	{
		g_time_osd = false;
	}
	//read video resolution
	//////////////////////////////////////////////
	if(!strcmp(video->dpi1, "720"))
	{
		g_resolution = VIDEO_MODE_720P;
	}
	else if (!strcmp(video->dpi1, "DVC"))
	{
		g_resolution = VIDEO_MODE_DVC;
	}
	else if (!strcmp(video->dpi1, "svga"))
	{
		g_resolution = VIDEO_MODE_SVGA;
	}
	else if (!strcmp(video->dpi1, "vga"))
	{
		g_resolution = VIDEO_MODE_VGA;
	}	
	else if (!strcmp(video->dpi1, "qvga"))
	{
		g_resolution = VIDEO_MODE_QVGA;
	}
	else if (!strcmp(video->dpi1, "d1"))
	{
		g_resolution = VIDEO_MODE_D1;
	}	
	else fprintf(stdout,"No support resolution\n");
		
	IniSetting_destroy();
	return 0;
}
/**
 * @brief Write osd position camera ini file
 *
 */
static int write_osd_ini()
{
	int ret = -1;
	
	IniSetting_init();
	#ifdef TPOLL_DBG
	fprintf(stdout,"Call %s\n", __func__);
	#endif
	//struct picture_info *picture;	// 
	
	if(t_osd != HIDE) {
		picture->osd_name = "REC";
		switch (t_osd) {
	
			case LEFT_UP: 
				picture->osd_place = "left_up";
			break;
			
			case RIGHT_UP:
				picture->osd_place = "right_up";
			break;
			
			case LEFT_DOWN:
				picture->osd_place = "left_down";
			break;
			
			case RIGHT_DOWN:
				picture->osd_place = "right_down";
			break;
	
			default:
			perror("Receive osd position error\n");
			break;
		}
	}
	else {
		picture->osd_name = 0;
		picture->osd_place = 0;
	}

	if(t_time_osd) {
		picture->osd_time = "show";
	}
		else {
			picture->osd_time = "hide";
			
		}
	
	ret += IniSetting_SetPictureInfo(picture); 	//return 0 if success, else -1
	ret += IniSetting_save(); 					//return 1 if success
	
	IniSetting_destroy();
	//fprintf(stdout,"%s: ret=%d\n", __func__, ret);
	return ret;
}

/**
 * @brief Write videoformat camera ini file
 *
 */
static int write_video_ini()
{
	int ret = -1;
	
	IniSetting_init();
	#ifdef TPOLL_DBG
	fprintf(stdout,"Call %s\n", __func__);
	#endif
	//struct picture_info *picture;	// 
	
		switch (t_resolution) {
	
			case VIDEO_MODE_QVGA: 
				video->dpi1 = "qvga";
			break;
			
			case VIDEO_MODE_VGA:
				video->dpi1 = "vga";
			break;
			
			case VIDEO_MODE_D1:
				video->dpi1 = "d1";
			break;
			
			case VIDEO_MODE_720P:
				video->dpi1 = "720";
			break;
			
			case VIDEO_MODE_DVC:
				video->dpi1 = "DVC";
			break;		
			
			case VIDEO_MODE_SVGA:
				video->dpi1 = "svga";
			break;					
	
			default:
			perror("Receive videoformat error\n");
			break;
		}
			
	ret += IniSetting_SetVideoResolution(video); 	//return 0 if success, else -1
	ret += IniSetting_save(); 					//return 1 if success
	
	IniSetting_destroy();
	//fprintf(stdout,"%s: ret=%d\n", __func__, ret);
	return ret;
}

/**
 * @brief Exit from the func
 * 
 */
static void tpoll_exit(void)
{
	#ifdef TPOLL_DBG
	fprintf(stdout,"--%s--\n", __func__);
	#endif
	if(pid != -1){		//if child process work, kill his
		kill(pid, SIGTERM);
		wait(NULL);
	}
	if(sock)
		close(sock);
	serial_close(tty);
	serial_destroy(tty);
	
}

