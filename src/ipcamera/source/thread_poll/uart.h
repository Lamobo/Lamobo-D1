
#ifndef SERIAL_H
#define SERIAL_H

#define BUFF_SIZE 512
#define POLL_TIMEOUT 2000

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct serial_s serial_t;

/**
 * Create the serial structure.
 * Convenience method to allocate memory
 * and instantiate objects.
 * @return serial structure.
 */
serial_t* serial_create();

/**
 * Destroy the serial structure
 */
void serial_destroy(serial_t* s);

/**
 * Connect to a serial device.
 * @param s - serial structure.
 * @param device - serial device name.
 * @param baud - baud rate for connection.
 * @return -ve on error, 0 on success.
 */
int serial_connect(serial_t* s, char device[], int baud);

/**
 * Send data.
 * @param s - serial structure.
 * @param data - character array to transmit.
 * @param length - size of the data array.
 */
int serial_send(serial_t* s, uint8_t data[], int length);

/**
 * Send a single character.
 * @param s - serial structure.
 * @param data - single character to be sent.
 */
void serial_put(serial_t* s, uint8_t data);

/**
 * Determine how much data is available
 * in the serial buffer.
 * @param s - serial structure.
 * @return number of characters available.
 */
int serial_available(serial_t* s);

/**
 * Fetch one char from the serial buffer.
 * @param s - serial structure.
 * @return character. Null if empty.
 */
char serial_get(serial_t* s);

/**
 * Fetch one char from the serial buffer.
 * Blocks until data becomes available.
 * @param s - serial structure.
 * @return character.
 */
char serial_blocking_get(serial_t* s);

/**
 * Clear the serial buffer.
 * @param s - serial structure.
 */
void serial_clear(serial_t* s);

/**
 * Close the serial port.
 * @param s - serial structure.
 * @return value of close().
 */
int serial_close(serial_t* s);

#ifdef __cplusplus
}
#endif

#endif
