#ifndef _ETHERNET_OPERATION_H_
#define _ETHERNET_OPERATION_H_

#define FLAG(_off)				((unsigned int)1 << (_off))
#define BIT_SET(_val, _off)		( (_val) |= FLAG(_off) ) //set the bit
#define BIT_CLEAR(_val, _off)	( (_val) &= ~FLAG(_off) ) // clear the bit
#define BIT_TEST(_val, _off)	(0 != ( (_val) & FLAG(_off) ) ) //test the bit
#endif
