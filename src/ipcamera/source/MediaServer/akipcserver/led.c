#include "stdlib.h"

#include "led.h"
#include "video_process.h"

static int record_flag = 0;
static int view_flag[2] = {0, 0};

void setled_record_start( int index )
{
	if( 0 == view_flag[0] && 0 == view_flag[1])
	{
		system("/etc/init.d/wifi_led.sh vedio_led off");
		system("/etc/init.d/wifi_led.sh vedio_led on");
	}
	else
	{
		system("/etc/init.d/wifi_led.sh vedio_led blink 250 250 ");
	}
	
	record_flag = index + 1;
	video_process_SetRecordFlag(record_flag);
}

void setled_record_stop( int index )
{
	if( 0 == view_flag[0] && 0 == view_flag[1])
	{
		system("/etc/init.d/wifi_led.sh vedio_led off");
	}
	else
	{
		system("/etc/init.d/wifi_led.sh vedio_led blink 1000 1000 ");
	}
	
	record_flag = 0;
	video_process_SetRecordFlag(record_flag);
}

void setled_view_start( int index )
{
	view_flag[index] = 1;
	if ( 0 == record_flag )
	{
		system("/etc/init.d/wifi_led.sh vedio_led blink 1000 1000 ");
	}
	else
	{
		system("/etc/init.d/wifi_led.sh vedio_led blink 250 250 ");
	}

	if(view_flag[0] == 1 && view_flag[1] == 0)
		video_process_SetViewFlag(1);
	if(view_flag[0] == 0 && view_flag[1] == 1)
		video_process_SetViewFlag(2);
	if(view_flag[0] == 1 && view_flag[1] == 1)
		video_process_SetViewFlag(3);
}

void setled_view_stop( int index )
{
	view_flag[index] = 0;
	if ( 0 == record_flag)
	{
		if(view_flag[0] == 0 && view_flag[1] == 0)
			system("/etc/init.d/wifi_led.sh vedio_led off");
		else
		{
			system("/etc/init.d/wifi_led.sh vedio_led off");
			system("/etc/init.d/wifi_led.sh vedio_led on");
		}
	}
	else
	{
		if(view_flag[0] == 0 && view_flag[1] == 0)
		{
			system("/etc/init.d/wifi_led.sh vedio_led off");
			system("/etc/init.d/wifi_led.sh vedio_led on");
		}
		else
		{
			system("/etc/init.d/wifi_led.sh vedio_led blink 250 250 ");
		}
	}

	if(view_flag[0] == 0 && view_flag[1] == 0)
		video_process_SetViewFlag(0);
	if(view_flag[0] == 1 && view_flag[1] == 0)
		video_process_SetViewFlag(1);
	if(view_flag[0] == 0 && view_flag[1] == 1)
		video_process_SetViewFlag(2);
}

void setled_off(void)
{
	system("/etc/init.d/wifi_led.sh vedio_led off");
}
