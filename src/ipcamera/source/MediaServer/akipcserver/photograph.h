#ifndef __PHOTOGRAPH_H__
#define __PHOTOGRAPH_H__

#ifdef __cplusplus
extern "C"
{
#endif

int photograph( void *pbuf, int size);
void Init_photograph( void );
void close_encode( void );


#ifdef __cplusplus
}
#endif

#endif