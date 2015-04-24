#ifndef CGI_ANYKA_H
#define CGI_ANYKA_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include "iniparser.h"

#define APCONF			"/etc/hostapd.conf"
#define PASSWD 			"/etc/hostapd.conf"			// password file 
#define AP_NAME			"/etc/hostapd.conf"			// ap name file
//#define CAMERA			"/etc/camera.ini"
#define CAMERA			"camera.ini"
#define SETTING			"setting"						// setting path
#define LOGIN			"login"						// login path
#define AP				"ap_list"						// scan ap list
#define MY_AP			"my_ap"						// myself ap 
#define WIFI_CONNET	"/etc/init.d/wifi_connect.sh"
#define DEVICE_SAVE		"/etc/init.d/device_save.sh"
#define CONSOLE			"/dev/console"
#define SRCOUT			"/dev/console 2>&1"
#define CNAME			"login"						// cookie name
#define LOK				"ok"							// login ok
#define LFAIL			"fail"						// login fail
#define TITLE			"vasens IP Camera"			// web title
#define WELCOME		"vasens IP Camera"			// 

#define OPEN			0
#define WEP				1
#define WPA				2

#define AP_SIZE 			8096
#define LINE 				1024
#define LEN				81

#define LEN32			32
#define LEN64			64
#define LEN96			96

#define LOGIN_DEBUG 1
#define SETTING_DEBUG 1
#define CGI_DEBUG 1

#ifdef LOGIN_DEBUG
	#define loginDebug(fmt...) 		printf(fmt)
#else
	#define loginDebug(fmt...)		 do {}while(0)
#endif

#ifdef SETTING_DEBUG
	#define settingDebug(fmt...) 	printf(fmt)
#else
	#define settingDebug(fmt...)	 do {}while(0)
#endif

#ifdef CGI_DEBUG
	#define cgiDebug(fmt...)		printf(fmt)
#else
	#define cgiDebug(fmt...)		do {}while(0)
#endif

#define print_error(fmt...)			fprintf(stderr, fmt)

#define print_error_2html(fmt...)	\
	do{	\
		cgiHeaderContentType("text/html");	\
		fprintf(cgiOut, "<HTML><HEAD><TITLE>ERROR</TITLE></HEAD>\n" \
	                		  "<BODY><H1>Error Occurred</H1>\n");	\
		fprintf(cgiOut, fmt);	\
		fprintf(cgiOut, "</BODY></HTML>\n");	\
		fflush(cgiOut);	\
	}while (0)


struct ap {
	int index;
	int security;
	char address[LEN];
	char ssid[LEN];
	char password[LEN];
	char protocol[LEN];
	char mode[LEN];
	char frequency[LEN];
	char en_key[LEN];
	char bit_rates[LEN];
	char sig_level[LEN];
};

struct station {
	int result;
	int security;
	int keyindex;
	char ssid[LEN];
	char wpapw[LEN];
	char wepkey[LEN];
};

struct wifi {
	int index;
	int security;
	int nid;
	char ssid[LEN];
};

struct ap_shop {
	int ap_num;
	struct ap ap_list[LEN32];
};


struct global_info{
	char *title;
	char *welcome;
};
struct ethernet_info{
	char *dhcp;
	char *ipaddr;
	char *netmask;
	char *gateway;
	char *firstdns;
	char *backdns;
};
struct wireless_info{
	char *open;
	char *ssid;
	char *mode;
	char *security;
	char *password;
	struct ap_shop ap_info;
};

#if 0
struct recoder_info{
	char *repeat;
	char *policy;
	char *week;
	char *time1_start;
	char *time1_end;
	char *time2_start;
	char *time2_end;
	char *time3_start;
	char *time3_end;
	char *time4_start;
	char *time4_end;
	char *free_space;
};
#endif

struct picture_info
{
	char *video_index;
	char *el_hz;
	char *osd_name;
	char *osd_place;
	char *osd_time;
};

struct recoder_info
{
	char *video_index;
	char *length;
	char *time;
};

struct user_info{
	char *name;
	char *passwd;
	char *confirmpw;
	char *timezone;
};
struct system_info{
	char *sn;
	char *pw;
	char *product;
	char *type;
	char *serverip;
	char *freq;
	char *serveradd;
	char *port;
};

struct video_info
{
	char *format1;
	char *dpi1;
	char *kbps1;
	char *kbps_mode1;
	char *group1;
	char *fps1;
	char *format2;
	char *dpi2;
	char *kbps2;
	char *kbps_mode2;
	char *quality;
	char *group2;
	char *fps2;
	char *video_kbps;
};

struct setting_info{
	char mode[LEN32];
	dictionary *ini;
	struct global_info global;
	struct ethernet_info ethernet;
	struct wireless_info wireless;
	struct recoder_info recoder;
	struct user_info user;
	struct system_info system;
	struct video_info video;
	struct picture_info picture;
};

struct occ_info
{
	char *start_xpos1;
	char *start_ypos1;
	char *end_xpos1;
	char *end_ypos1;
	char *enable1;
	char *start_xpos2;
	char *start_ypos2;
	char *end_xpos2;
	char *end_ypos2;
	char *enable2;
	char *start_xpos3;
	char *start_ypos3;
	char *end_xpos3;
	char *end_ypos3;
	char *enable3;
	char *start_xpos4;
	char *start_ypos4;
	char *end_xpos4;
	char *end_ypos4;
	char *enable4;
};

struct md_info
{
	char *matrix_high;
	char *matrix_low;
	char *nSensitivity;
	char *on;
};

struct isp_info
{
	char *nContrast;
	char *nSaturation;
	char *nBrightness;
};

struct akisetting_info{
	char mode[LEN32];
	dictionary *ini;
	struct occ_info occ;
	struct md_info md;
	struct isp_info isp;
};


#if 0
void cgiRedirect(char *page);
int cgiIsMobile(void);
int cgiGetVal(const char *str, const char *name, char *val);
int cgiCheckSecurity(char *buf);
int cgiGetMyAp(struct ap *ap);
void cgiScanOtherAp2File(void);
void cgiScanMyAp2File(void);
int cgiCheckLogin(void);
void cgiRestoreStdout(void);
void cgiRestoreStderr(void);
void cgiRestoreStdin(void);
#endif

#ifdef __cplusplus
} /* end extern "C" */
#endif


#endif	//CGI_ANYKA_H
