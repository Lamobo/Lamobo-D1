#ifndef __LED_H__
#define __LED_H__

#ifdef __cplusplus
extern "C"{
#endif
void setled_off(void);
void setled_record_start(int index);
void setled_record_stop(int index);
void setled_view_start(int index);
void setled_view_stop(int index);

#ifdef __cplusplus
	}
#endif

#endif
