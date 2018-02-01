/**
  \file 
	\brief Serial operation 
	\author gist.github.com/ryankurte/8143058

*/ 
#include "uart.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>

/**
 * @struct Serial device structure.
 * 
 * Encapsulates a serial connection.
 */
struct serial_s {
    int fd;                  ///< Connection file descriptor.
    int state;               ///< Signifies connection state.
    int running;             ///< Signifies thread state.

    char rxbuff[BUFF_SIZE];  ///< Buffer for RX data.
    int start;          	///< Pointers to start of buffer.
    int end;          		///< Pointers to end of buffer.

    pthread_t rx_thread;     ///< Listening thread.
};

// ---------------        Internal Functions        ---------------

static int serial_resolve_baud(int baud);

/**
 * @brief Recieve data.
 * 
 * Retrieves data from the serial device.
 * @param s - serial structure.
 * @param data - pointer to a buffer to read data into.
 * @param maxLength - size of input buffer.
 * @return amount of data recieved.
 */
static int serial_recieve(serial_t* obj, uint8_t data[], int maxLength);

/**
 * @brief Serial Listener Thread.
 * 
 * This blocks waiting for data to be recieved from the serial device,
 * and calls the serial_callback method with appropriate context when
 * data is recieved.
 * Exits when close method is called, or serial error occurs.
 * @param param - context passed from thread instantiation.
 */
static void *serial_data_listener(void *param);

/**
 * @brief Start the serial threads.
 * 
 * This spawns the listening and transmitting threads
 * if they are not already running.
 * @param s - serial structure.
 * @return 0 on success, or -1 on error.
 */
static int serial_start(serial_t* s);

/**
 * @brief Stop serial listener thread.
 * @param s - serial structure.
 * @return 0;
 */
static int serial_stop(serial_t* s);

/**
 * @brief Callback to handle recieved data.
 * 
 * Puts recieved data into the rx buffer.
 * @param s - serial structure.
 * @param data - data to be stored.
 * @param length - length of recieved data.
 */
static void serial_rx_callback(serial_t* s, char data[], int length);

// Put character in rx buffer.
static int buffer_put(serial_t* s, char c)
{
    //if there is space in the buffer
    if ( s->end != ((s->start + BUFF_SIZE - 1) % BUFF_SIZE)) {
        s->rxbuff[s->end] = c;
        s->end ++;
        s->end = s->end % BUFF_SIZE;
        //printf("Put: %x start: %d, end: %d\r\n", c, s->start, s->end);
        return 0;        //No error
    } else {
        //buffer is full, this is a bad state
        return -1;        //Report error
    }
}

// Get character from rx buffer.
static char buffer_get(serial_t* s)
{
    char c = (char)0;
    //if there is data to process
    if (s->end != s->start) {
        c = (s->rxbuff[s->start]);
        s->start ++;
        //wrap around
        s->start = s->start % BUFF_SIZE;
    } else {
    }
    //printf("Get: %x start: %d, end: %d\r\n", c, s->start, s->end);
    return c;
}

//Get data available in the rx buffer.
static int buffer_available(serial_t* s)
{
    return (s->end - s->start + BUFF_SIZE) % BUFF_SIZE;
}

// ---------------        External Functions        ---------------

//Create serial object.
serial_t* serial_create()
{
    //Allocate serial object.
    serial_t* s = malloc(sizeof(serial_t));
    //Reconfigure buffer object.
    s->start = 0;
    s->end = 0;
    //Return pointer.
    return s;
}


void serial_destroy(serial_t* s)
{
    free(s);
}


//Connect to serial device.
int serial_connect(serial_t* s, char device[], int baud)
{
    struct termios oldtio;

    // Resolve baud.
    int speed = serial_resolve_baud(baud);
    if (speed < 0) {
        printf("Error: Baud rate not recognized.\r\n");
        return -1;
    }

    //Open device.
    s->fd = open(device, O_RDWR | O_NOCTTY);
    //Catch file open error.
    if (s->fd < 0) {
        perror(device);
        return -2;
    }
    //Retrieve settings.
    tcgetattr(s->fd, &oldtio);
    //Set baud rate.
    cfsetspeed(&oldtio, speed);
    //Flush cache.
    tcflush(s->fd, TCIFLUSH);
    //Apply settings.
    tcsetattr(s->fd, TCSANOW, &oldtio);

    //Start listener thread.
    int res = serial_start(s);
    //Catch error.
    if (res < 0) {
        printf("Error: serial thread could not be spawned\r\n");
        return -3;
    }

    //Indicate connection was successful.
    s->state = 1;
    return 0;
}

//Send data.
int serial_send(serial_t* s, uint8_t data[], int length)
{
    int res = write(s->fd, data, length);
    return res;
}

void serial_put(serial_t* s, uint8_t data)
{
    char arr[1];
    arr[0] = data;
    write(s->fd, arr, 1);
}

//Determine characters available.
int serial_available(serial_t* s)
{
    return buffer_available(s);
}

//Fetch a character.
char serial_get(serial_t* s)
{
    char c = buffer_get(s);

    return c;
}

char serial_blocking_get(serial_t* s)
{
    while (serial_available(s) == 0);
    return serial_get(s);
}

void serial_clear(serial_t* s)
{
    //Clear the buffer.
    while (buffer_available(s)) {
        buffer_get(s);
    }
}

//Close serial port.
int serial_close(serial_t* s)
{
    //Stop thread.
    serial_stop(s);
    return 0;
}

// ---------------        Internal Functions        --------------

//Stop serial listener thread.
static int serial_stop(serial_t* s)
{
    s->running = 0;
    return 0;
}

// Resolves standard baud rates to linux constants.
static int serial_resolve_baud(int baud)
{
    int speed;
    // Switch common baud rates to temios constants.
    switch (baud) {
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
    case 57600:
        speed = B57600;
        break;
    case 115200:
        speed = B115200;
        break;
    default:
        speed = -1;
        break;
    }
    // Return.
    return speed;
}

// Start serial listener.
static int serial_start(serial_t* s)
{
    //Only start if it is not currently running.
    if (s->running != 1) {
        //Set running.
        s->running = 1;
        //Spawn thread.
        int res;
        res = pthread_create(&s->rx_thread, NULL, serial_data_listener, (void*) s);
        if (res != 0) {
            return -2;
        }
        //Return result.
        return 0;
    } else {
        return -1;
    }
}

//Recieve data.
static int serial_recieve(serial_t* s, uint8_t data[], int maxLength)
{
    return read(s->fd, data, maxLength);
}

//Callback to store data in buffer.
static void serial_rx_callback(serial_t* s, char data[], int length)
{
    //Put data into buffer.
    int i;
    //Put data into buffer.
    for (i = 0; i < length; i++) {
        buffer_put(s, data[i]);
    }

}

//Serial data listener thread.
static void *serial_data_listener(void *param)
{
    int res = 0;
    int err = 0;
    struct pollfd ufds;
    uint8_t buff[BUFF_SIZE];

    //Retrieve paramaters and store locally.
    serial_t* serial = (serial_t*) param;
    int fd = serial->fd;

    //Set up poll file descriptors.
    ufds.fd = fd;        //Attach socket to watch.
    ufds.events = POLLIN;        //Set events to notify on.

    //Run until ended.
    while (serial->running != 0) {
        //Poll socket for data.
        res = poll(&ufds, 1, POLL_TIMEOUT);
        //If data was recieved.
        if (res > 0) {
            //Fetch the data.
            int count = serial_recieve(serial, buff, BUFF_SIZE - 1);
            //If data was recieved.
            if (count > 0) {
                //Pad end of buffer to ensure there is a termination symbol.
                buff[count] = '\0';
                // Call the serial callback.
                serial_rx_callback(serial, (char *)buff, count);
                //If an error occured.
            } else if (count < 0) {
                //Inform user and exit thread.
                printf("Error: Serial disconnect\r\n");
                err = 1;
                break;
            }
            //If there was an error.
        } else if (res < 0) {
            //Inform user and exit thread.
            printf("Error: Polling error in serial thread");
            err = 1;
            break;
        }
        //Otherwise, keep going around.
    }
    //If there was an error, close socket.
    if (err) {
        serial_close(serial);
        //raise(SIGLOST);
    }
    //Close file.
    res = close(serial->fd);

    return NULL;
}
