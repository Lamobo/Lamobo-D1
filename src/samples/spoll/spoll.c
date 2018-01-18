#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <poll.h>
#include <pthread.h>
#include "fifo.h"

#define POLL_TIMEOUT 2000

//#include <sys/types.h>
//#include <sys/stat.h>

struct termios info, oldinfo;
int fdi,err;
const char s_in[]={"/dev/ttySAK1"};
int baud = B115200;

int main(void) {
	char txbuf[64];
	ssize_t cnt;
	int n;
	struct pollfd  poll_fd[1];
	FIFO8(32) s_fifo;
	FIFO_FLUSH(s_fifo);
	
	//open port
	fdi = open(s_in, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fdi < 0) {
     	perror(s_in);
     	exit(1);
	}
	//setup termios struct
   err = tcgetattr(fdi, &oldinfo);
   info.c_cflag = (CS8 | CREAD) & ~PARENB; /* 8N1 */
	//info.c_iflag |= IXON | IXOFF;
   info.c_iflag &= ~(IXON | IXOFF | IXANY);
	// info.c_lflag &= ~ICANON & ~ISIG;
   info.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
   info.c_oflag &= ~OPOST;
   
   err = cfsetospeed(&info, baud);
   err = cfsetispeed(&info, baud);
   tcflush(fdi, TCIFLUSH);
   err = tcsetattr(fdi, TCSANOW, &info);
	//___________________________________________________
   poll_fd[0].fd = fdi;
   poll_fd[0].events = POLLIN;
   poll_fd[0].revents = 0;
  

while(1){
	n = poll(poll_fd, 1, POLL_TIMEOUT); /* wait here */
    if (n > 0){
		cnt = read(poll_fd[0].fd, txbuf, sizeof(txbuf));
		if (cnt > 0) {	
			//txbuf[cnt]=0;
			//printf("%s", txbuf);
			//putchar('\n');
			int i;
			for (i=0; i < cnt; i++){
			//printf("%c", txbuf[i]);
			FIFO_PUSH(s_fifo, txbuf[i]); 
			}
		}
		poll_fd[0].revents = 0;
	}
    else if (n == 0){
		printf("no data within %d sec\n", POLL_TIMEOUT/1000);
		//continue;
	}
	else if (n < 0) {
		perror("poll");
		break;
    }
	sleep(1);
	
	if(!FIFO_IS_EMPTY(s_fifo)) {
		/*while(FIFO_COUNT(s_fifo)){
			printf("%c", FIFO_FRONT(s_fifo));
			FIFO_POP(s_fifo);
		}*/
		printf("%c", FIFO_FRONT(s_fifo));
		FIFO_POP(s_fifo);
	}
}
err = tcsetattr(fdi, TCSANOW, &oldinfo);
close(fdi);
exit(0);
} 

