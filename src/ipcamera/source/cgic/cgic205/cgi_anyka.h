/*
 *  Copyright (C) 2012 anyka <gao_wangsheng@anyka.oa>
 *  Create by gao_wangsheng  2013-5-6
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef CGI_ANYKA_H
#define CGI_ANYKA_H

#include <stdbool.h>
#include "iniparser.h"

#define APCONF			"/etc/hostapd.conf"
#define PASSWD 			"/etc/hostapd.conf"			// password file 
#define AP_NAME			"/etc/hostapd.conf"			// ap name file
#define CAMERA			"/etc/jffs2/camera.ini"
#define SETTING			"setting"						// setting path
#define LOGIN			"login"						// login path
#define AP				"/var/ap_list"						// scan ap list
#define MY_AP			"/var/my_ap"						// myself ap 
#define WIFI_CONNET		"/etc/init.d/wifi_connect.sh"
#define IPC_RESTART		"/etc/init.d/restart_IPC.sh"
#define DEVICE_SAVE		"/etc/init.d/device_save.sh"
#define SYSTEM_RECOVER	"/etc/init.d/wifi_recover.sh sys"
#define CONSOLE			"/dev/console"
#define SRCOUT			"/dev/console 2>&1"
#define CNAME			"login"						// cookie name
#define LOK				"ok"							// login ok
#define LFAIL			"fail"						// login fail
#define TITLE			"vasens IP Camera"			// web title
#define WELCOME			"vasens IP Camera"			// 

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
	char *ssid;
	char *mode;
	char *security;
	char *password;
	struct ap_shop ap_info;
};

struct softap_info{
	char *ssid;
	char *password;
};

struct video{
	char *format;
	char *dpi;
	char *kbps;
	char *kbps_mode;
	char *quality;
	char *group;
	char *fps;
};
struct video_info{
	//char *auto_kbps;
	struct video video1;
	struct video video2;
	char *video_kbps;
};

struct picture_info{
	char *video_index;
	char *el_hz;
	char *osd_name;
	char *osd_place;
	char *osd_time;
};

struct recoder_info{
	char *video_index;
	char *length;
	char *time;
};
struct user_info{
	char *name;
	char *passwd;
	char *root_name;
	char *root_passwd;
};
struct system_info{
	char *serverip;
	char *serveradd;
	char *port;
};
struct setting_info{
	char mode[LEN32];
	int admin;
	dictionary *ini;
	struct global_info global;
	struct ethernet_info ethernet;
	struct wireless_info wireless;
	struct softap_info softap;
	struct video_info video;
	struct picture_info picture;
	struct recoder_info recoder;
	struct user_info user;
	struct system_info system;
};


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


#endif	//CGI_ANYKA_H
