/*
 * siso
 * like a breakout box for serial connections
 * put the computer running siso in the data
 * flow of a serial connection
 * PC -> serial -> PC(siso) -> serial -> device
 *
 * selectable items are
 * -b baud_rate
 * -f logfile
 * -i input_serial_device
 * -o output_serial_device
 *
 * default connection is 
 * 115200 8N1 ttyS0 to ttyS1
 *
 */

/* this is for snprintf (under debian) */
#define _ISOC99_SOURCE 1
#define _POSIX_SOURCE  1

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>
#include <signal.h>

#define DEFAULT_SERIAL_IN_PORT "/dev/pts/0"
#define DEFAULT_SERIAL_OUT_PORT "/dev/pts/0"

static int exit_flag = 0;

static void hdl (int sig)
{
	exit_flag = 1;
}

int set_baud(char* baud_s) {
  int b;
  int baud;

  b = atoi(baud_s);
  switch (b) {
  case 300 : baud = B300; break;
  case 1200 : baud = B1200; break;
  case 2400 : baud = B2400; break;
  case 9600 : baud = B9600; break;
  case 19200 : baud = B19200; break;
  case 38400 : baud = B38400; break;
  case 57600 : baud = B57600; break;
  case 115200 : baud = B115200; break;
  default : baud = 0;
  };
  return baud;
}

int main (int argc, char* argv[]) {
  int            err;
  int            fdi;
  int            fdo;
  int            fdl;
  int            opt;
  int            n;
  int            baud;
  char           buf[8];
  char           log[1024];
  char           s_in[128];
  char           s_out[128];
  char           buffer[2048];
  struct pollfd  poll_fd[2];
  struct termios info;
  
  struct sigaction act;
 
	memset (&act, '\0', sizeof(act));
	act.sa_handler = &hdl;
	if (sigaction(SIGINT, &act, NULL) < 0) {
		perror ("sigaction");
		return 1;
	}
 



  openlog("SiSo", LOG_PID, LOG_USER);

  /* setup defaults */
  baud = B115200;
  snprintf(s_in, sizeof(s_in), "%s", DEFAULT_SERIAL_IN_PORT);
  snprintf(s_out, sizeof(s_out), "%s", DEFAULT_SERIAL_OUT_PORT);
  for(opt=getopt(argc, argv, "f:i:o:b:"); opt != -1;
      opt=getopt(argc, argv, "f:i:o:b:")) {
    switch (opt) {
    case 'b' : { /* set baud rate */
      n = snprintf(buf, sizeof(buf), "%s", optarg);
      if (n < 0 || n > sizeof(buf)) {
        syslog(LOG_ERR, "couldn't convert -b option into a baud rate");
        return n;
      }
      baud = set_baud(buf);
      if (baud == 0) {
        syslog(LOG_ERR, "baud rate [%s] not supported", buf);
        return 0;
      }
    }; break;
    case 'f' : { /* output file */
      n = snprintf(log, sizeof(log), "%s", optarg);
      if (n < 0 || n > sizeof(log)) {
        syslog(LOG_ERR, "output filename truncated, longer than %d bytes", sizeof(log));
        closelog();
        return n;
      }
    }; break;
    case 'i' : { /* serial input device */
      n = snprintf(s_in, sizeof(s_in), "%s", optarg);
      if (n < 0 || n > sizeof(s_in)) {
        syslog(LOG_ERR, "output filename truncated, longer than %d bytes", 
               sizeof(s_in));
        closelog();
        return n;
      }
    }; break;
    case 'o' : { /* serial output device */
      n = snprintf(s_in, sizeof(s_out), "%s", optarg);
      if (n < 0 || n > sizeof(s_out)) {
        syslog(LOG_ERR, "output filename truncated, longer than %d bytes", 
               sizeof(s_out));
        closelog();
        return n;
      }
    }; break;
    }; /* end switch */
  } /* end for */
  fdi = open (s_in, O_RDWR);
  if (fdi < 0) {
    syslog(LOG_ERR, "couldn't open %s : %m", s_in);
    exit(0);
  }
  fdo = open (s_out, O_RDWR);
  if (fdo < 0) {
    syslog(LOG_ERR, "couldn't open %s : %m", s_out);
    exit(0);
  }
  fdl = open(log, O_RDWR|O_CREAT);
  if (fdl < 0) {
    syslog(LOG_ERR, "couldn't open %s : %m", log);
    exit(0);
  }
  
  /* set up modem connection */
  err = tcgetattr(fdi, &info);
  info.c_cflag = (CS8 | CREAD) & ~PARENB; /* 8N1 */
  info.c_iflag |= IXON | IXOFF;
  info.c_lflag &= ~ICANON & ~ISIG;
  err = cfsetospeed(&info, baud);
  err = cfsetispeed(&info, baud);
  err = tcsetattr(fdi, TCSANOW, &info);

  /* set up serial connection */
  err = tcgetattr(fdo, &info);
  info.c_cflag = (CS8 | CREAD) & ~PARENB; /* 8N1 */
  info.c_iflag |= IXON | IXOFF;
  info.c_lflag &= ~ICANON & ~ISIG;
  err = cfsetospeed(&info, baud);
  err = cfsetispeed(&info, baud);
  err = tcsetattr(fdo, TCSANOW, &info);

  /* set up poll structure */
  poll_fd[0].fd = fdi;
  poll_fd[1].fd = fdo;
  poll_fd[0].events = POLLIN;
  poll_fd[1].events = POLLIN;
  poll_fd[0].revents = 0;
  poll_fd[1].revents = 0;

  /* main loop */
  while (!exit_flag) {
    n = poll(poll_fd, 2, -1); /* wait here */
    if (n < 0) {
      break;
    }
    if (poll_fd[0].revents & POLLIN) {
      n = read(poll_fd[0].fd, buffer, sizeof(buffer));
      if (n > 0) {
        write(poll_fd[1].fd, buffer, n);
        write(fdl, buffer, n);
      }
      poll_fd[0].revents = 0;
    }
    if (poll_fd[1].revents & POLLIN) {
      n = read(poll_fd[1].fd, buffer, sizeof(buffer));
      if (n > 0) {
        write(poll_fd[0].fd, buffer, n);
        write(fdl, buffer, n);
      }
      poll_fd[1].revents = 0;
    }
  }
  close(fdl);
  close(fdi);
  close(fdo);
  exit(0);
}
