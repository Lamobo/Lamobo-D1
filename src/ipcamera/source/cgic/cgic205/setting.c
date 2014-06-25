/*
 *  setting.c, a wifi setting page and cgi
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "cgic.h"
#include "cgi_anyka.h"

#define ETH_HTML	0
#define WIFI_HTML	1
#define REC_HTML	2
#define USER_HTML	3
#define REV_HTML	4
#define SYS_HTML	5
#define UPD_HTML	6
#define APP_HTML	7
#define KER_HTML	8
#define PIC_HTML	9
#define VID_HTML	10
#define SAP_HTML	11
#define REBOOT_HTML	12

#define SEM_PROJ_ID	0x23
#define SEM_NUMS		6

#define KERNEL_UPDATE_OP						0
#define KERNEL_UPDATE_OP_DOWNLOAD		1
#define KERNEL_UPDATE_SD						2
#define KERNEL_UPDATE_DOWNLOAD			3
#define KERNEL_UPDATE_CAN_BURN_KER	4
#define KERNEL_UPDATE_FILE					5

key_t sem_key;
int sem_id;

union semun{
	int								val;		//semctl SETVAL
	struct semid_ds *	buf;		//semctl IPC_STAT, IPC_SET
	unsigned short	*	array; 	//Array for GETALL, SETALL
	struct seminfo	* __buf;	//IPC_INFO
};

static char *safe[] = {
	"NONE",
	"WEP",
	"WPA/WPA2 PSK"
};
static char *wifi_mode[] ={
	"Infra",
	"Ad-Hoc"
};

static char *format[] = {
	"h264",
	"mjpeg",
};
static char *dpi[] = {
	"720",
};
static char *dpi2[] = {
	"d1",
	"vga",
	"qvga",
};
static char *code_stream[] = {
	"1",
	"2",
};

static char *elhz[] = {
	"50",
	"60",
};

static char *osd_place[] = {
	"left_up",
	"right_up",
	"left_down",
	"right_down",
};

static char *osd_time[] = {
	"hide",
	"show",
};

static char *time[] = {
	"2",
	"5",
	"15",
	"30",
};

static char * quality[] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
};

/**
 * *  @brief       get an ap infom from buf
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   buf, ap
 * *  @return      non zero on valid ap
 * */
static int cgiGetOneAp(char *buf, struct ap *ap)
{
	cgiGetVal(buf, "Address: ", ap->address);
	cgiGetVal(buf, "ESSID:\"", ap->ssid);
	cgiGetVal(buf, "Protocol:", ap->protocol);
	cgiGetVal(buf, "Mode:", ap->mode);
	cgiGetVal(buf, "Frequency:", ap->frequency);
	cgiGetVal(buf, "Encryption key:", ap->en_key);
	cgiGetVal(buf, "Bit Rates:", ap->bit_rates);
	cgiGetVal(buf, "level=", ap->sig_level);
	ap->security = cgiCheckSecurity(buf);
	return strlen(ap->ssid);
}

/**
 * *  @brief       get one ap buf
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   info
 * *  @return      zero on success
 * */
static int cgiAllocOneAp(struct ap_shop *info)
{
	info->ap_num++;
	if (info->ap_num >= sizeof(info->ap_list)/sizeof(struct ap))
	{
		info->ap_num--;
		print_error("%s->%s:ap list=%d is full!", __FILE__, __func__, info->ap_num);
		return -1;
	}
	return 0;
}

/**
 * *  @brief       init one ap buf
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   info
 * *  @return      none
 * */
static void cgiInitApInfo(struct ap_shop *info)
{
	memset(info, 0, sizeof(struct ap_shop));
	info->ap_num = -1;
}

/**
 * *  @brief       get current connecting ap info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   info
 * *  @return      none zero on valid ap
 * */
static int cgiScanConnectAp(struct ap_shop *info)
{
	char buf[LINE];
	FILE *filp;
	struct ap *ap;

	cgiDebug("%s: \n", __func__);

	filp = popen("/usr/sbin/wpa_cli -iwlan0 status", "r");
	if (NULL == filp){
		print_error("%s->%s: popen fail!\n", __FILE__, __func__);
		return -2;
	}

	if (cgiAllocOneAp(info)){
		print_error("%s->%s: cgi Alloc One Ap fail!\n", __FILE__, __func__);
		pclose(filp);
		return -1;
	}
	ap = &info->ap_list[info->ap_num];

	fgets(buf, sizeof(buf)-1, filp);
	memset(buf, '\0', sizeof(buf));
	fread(buf, sizeof(char), sizeof(buf)-1, filp);
	cgiGetVal(buf, "address=", ap->address);
	cgiGetVal(buf, "ssid=", ap->ssid);
	settingDebug("%s: Connect ssid is %s !\n", __func__, ap->ssid);
	ap->security = cgiCheckSecurity(buf);

	pclose(filp);
	return strlen(ap->ssid);
}

/**
 * *  @brief       scan ap info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   info
 * *  @return      zero on success
 * */
static int cgiScanOtherAp(struct ap_shop *info)
{
	int error = 0;
	char line[LINE];
	char buffer[AP_SIZE];
	FILE *ap_file;
	struct ap ap_temp;

	cgiDebug("%s: Scanning other available wifi AP...\n", __func__);

	cgiScanOtherAp2File();
	ap_file = fopen(AP, "r");
	if (NULL == ap_file)
	{
		print_error("%s->%s: open file %s fail!\n", __FILE__, __func__, AP);
		goto fail;
	}

	memset(line, '\0', sizeof(line));

	while ((!feof(ap_file)) && (strstr(line, "Address:") == NULL))
	{
		fgets(line, LINE, ap_file);
	}

	for (; (!feof(ap_file)) && (strstr(line, "Address:") != NULL); )
	{
		memset(buffer, '\0', sizeof(buffer));
		strcat(buffer, line);

		// get one AP info to buffer
		for (fgets(line, LINE, ap_file); (!feof(ap_file)) && (strstr(line, "Address:") == NULL); /*nul*/)
		{
			strcat(buffer, line);
			fgets(line, LINE, ap_file);
		}

		// copy one ap info to list
		memset(&ap_temp, 0, sizeof(struct ap));
		if (cgiGetOneAp(buffer, &ap_temp))
		{
			error = cgiAllocOneAp(info);
			if (error)
			{
				print_error("%s->%s: cgi Alloc One Ap fail!\n", __FILE__, __func__);
				fclose(ap_file);
				goto fail;
			}

			memcpy(&info->ap_list[info->ap_num], &ap_temp, sizeof(struct ap));
			info->ap_list[info->ap_num].index = info->ap_num;
		}
	}

	fclose(ap_file);
	return 0;
fail:
	return -1;
}

/**
 * *  @brief       get config from ini file
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void loadSettingInfo(struct setting_info *setting)
{
	//global info
	setting->global.title = iniparser_getstring(setting->ini, "global:title", NULL);
	setting->global.welcome = iniparser_getstring(setting->ini, "global:welcome", NULL);

	//ethernet info
	setting->ethernet.dhcp = iniparser_getstring(setting->ini, "ethernet:dhcp", NULL);
	setting->ethernet.ipaddr = iniparser_getstring(setting->ini, "ethernet:ipaddr", NULL);
	setting->ethernet.netmask = iniparser_getstring(setting->ini, "ethernet:netmask", NULL);
	setting->ethernet.gateway = iniparser_getstring(setting->ini, "ethernet:gateway", NULL);
	setting->ethernet.firstdns = iniparser_getstring(setting->ini, "ethernet:firstdns", NULL);
	setting->ethernet.backdns = iniparser_getstring(setting->ini, "ethernet:backdns", NULL);

	//wireless info
	setting->wireless.ssid = iniparser_getstring(setting->ini, "wireless:ssid", NULL);
	setting->wireless.mode = iniparser_getstring(setting->ini, "wireless:mode", NULL);
	setting->wireless.security = iniparser_getstring(setting->ini, "wireless:security", NULL);
	setting->wireless.password = iniparser_getstring(setting->ini, "wireless:password", NULL);

	//softap info
	setting->softap.ssid = iniparser_getstring(setting->ini, "softap:s_ssid", NULL);
	setting->softap.password = iniparser_getstring(setting->ini, "softap:s_password", NULL);

	//video info
	//setting->video.auto_kbps = iniparser_getstring(setting->ini, "video:auto_kbps", NULL);

	setting->video.video1.format = iniparser_getstring(setting->ini, "video:format1", NULL);
	setting->video.video1.dpi = iniparser_getstring(setting->ini, "video:dpi1", NULL);
	setting->video.video1.kbps_mode = iniparser_getstring(setting->ini, "video:kbps_mode1", NULL);
	setting->video.video1.kbps = iniparser_getstring(setting->ini, "video:kbps1", NULL);
	setting->video.video1.group = iniparser_getstring(setting->ini, "video:group1", NULL);
	setting->video.video1.fps = iniparser_getstring(setting->ini, "video:fps1", NULL);

	setting->video.video2.format = iniparser_getstring(setting->ini, "video:format2", NULL);
	setting->video.video2.dpi = iniparser_getstring(setting->ini, "video:dpi2", NULL);
	setting->video.video2.kbps_mode = iniparser_getstring(setting->ini, "video:kbps_mode2", NULL);
	setting->video.video2.kbps = iniparser_getstring(setting->ini, "video:kbps2", NULL);
	setting->video.video2.group = iniparser_getstring(setting->ini, "video:group2", NULL);
	setting->video.video2.quality = iniparser_getstring(setting->ini, "video:quality", NULL);
	setting->video.video2.fps = iniparser_getstring(setting->ini, "video:fps2", NULL);

	setting->video.video_kbps = iniparser_getstring(setting->ini, "video:video_kbps", NULL);

	//picture info
	setting->picture.video_index = iniparser_getstring(setting->ini, "picture:video_index", NULL);
	setting->picture.el_hz = iniparser_getstring(setting->ini, "picture:el_hz", NULL);
	setting->picture.osd_name = iniparser_getstring(setting->ini, "picture:osd_name", NULL);
	setting->picture.osd_place = iniparser_getstring(setting->ini, "picture:osd_place", NULL);
	setting->picture.osd_time = iniparser_getstring(setting->ini, "picture:osd_time", NULL);

	//recoder info
	setting->recoder.video_index = iniparser_getstring(setting->ini, "recoder:video_index", NULL);
	setting->recoder.length = iniparser_getstring(setting->ini, "recoder:length", NULL);
	setting->recoder.time = iniparser_getstring(setting->ini, "recoder:time", NULL);

	//user info
	setting->user.name = iniparser_getstring(setting->ini, "user:name", NULL);
	setting->user.passwd = iniparser_getstring(setting->ini, "user:passwd", NULL);
	setting->user.root_name = iniparser_getstring(setting->ini, "user:root_name", NULL);
	setting->user.root_passwd = iniparser_getstring(setting->ini, "user:root_passwd", NULL);

	//system info
	setting->system.serverip = iniparser_getstring(setting->ini, "system:serverip", NULL);
	setting->system.serveradd = iniparser_getstring(setting->ini, "system:serveradd", NULL);
	setting->system.port = iniparser_getstring(setting->ini, "system:port", NULL);

	//debug
//	iniparser_dump(setting->ini, stdout);
}

/**
 * *  @brief       store config to ini file
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      zero on success
 * */
static int storeSettingInfo(struct setting_info *setting)
{
	FILE *fp = NULL;

	cgiDebug("%s: ", __func__);
	loadSettingInfo(setting);
//	iniparser_dump(setting->ini, stdout);

	fp = fopen(CAMERA, "w");
	if (NULL == fp){
		print_error("%s:open %s file fail!", __func__, CAMERA);
		print_error_2html("%s:open %s file fail!", __func__, CAMERA);
		return -1;
	}

	iniparser_dump_ini(setting->ini, fp);
	fclose(fp);
	return 0;
}

/**
 * *  @brief       print main left html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting, which page
 * *  @return      none
 * */
static void printLeftIndexHtml(struct setting_info *setting, int index)
{
	char *on = "class=\"on\"";

	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf(cgiOut, "<head>\n");
	fprintf(cgiOut, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n");
	fprintf(cgiOut, "<title>%s</title>\n", setting->global.title);
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<link href=\"../css/style.css\" rel=\"stylesheet\" type=\"text/css\" />\n");
	fprintf(cgiOut, "<script type=\"text/javascript\" src=\"../js/jquery.js\" ></script>\n");
	fprintf(cgiOut, "<script type=\"text/javascript\" src=\"../js/js.js\" ></script>\n");
	fprintf(cgiOut, "</head>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<body>\n");
	fprintf(cgiOut, "<div class=\"main\">\n");
	fprintf(cgiOut, "<div class=\"fleft  main_left\">\n");
	fprintf(cgiOut, "<ul class=\"sidenav\">\n");
	fprintf(cgiOut, "<li>\n");
	fprintf(cgiOut, "<p class=\"main_title\"> Network Setup </p>\n");
	fprintf(cgiOut, "<p %s><a href=\"%s?mode=ethernet\"> Ethernet </a></p>\n",
			(ETH_HTML==index)?on:"", SETTING);
	fprintf(cgiOut, "<p %s><a href=\"%s?mode=wireless\"> Wireless </a></p>\n",
			(WIFI_HTML==index)?on:"", SETTING);
	fprintf(cgiOut, "<p %s><a href=\"%s?mode=softap\">SoftAP</a></p>\n",
			(SAP_HTML==index)?on:"", SETTING);
	fprintf(cgiOut, "</li>\n");
	fprintf(cgiOut, "<li>\n");
	fprintf(cgiOut, "<p class=\"main_title\"> Video Setup </p>\n");
	fprintf(cgiOut, "<p %s><a href=\"%s?mode=video\"> Video </a></p>\n",
			(VID_HTML==index)?on:"", SETTING);
	fprintf(cgiOut, "<p %s><a href=\"%s?mode=picture\"> Image </a></p>\n",
			(PIC_HTML==index)?on:"", SETTING);
	fprintf(cgiOut, "<p %s><a href=\"%s?mode=recoder\"> Recorder </a></p>\n",
			(REC_HTML==index)?on:"", SETTING);
	fprintf(cgiOut, "</li>\n");
	fprintf(cgiOut, "<li>\n");
	fprintf(cgiOut, "<p class=\"main_title\"> User Control </p>\n");
	fprintf(cgiOut, "<p %s><a href=\"%s?mode=user\"> User settings </a></p>\n",
			(USER_HTML==index)?on:"", SETTING);
	fprintf(cgiOut, "</li>\n");
	fprintf(cgiOut, "<li>\n");
	fprintf(cgiOut, "<p class=\"main_title\"> System Setup </p>\n");
	fprintf(cgiOut, "<p %s><a href=\"%s?mode=recover\"> Factory Reset </a></p>\n",
			(REV_HTML==index)?on:"", SETTING);
	fprintf(cgiOut, "<p %s><a href=\"%s?mode=update_app\"> App Update </a></p>\n",
			(APP_HTML==index)?on:"", SETTING);
	if (setting->admin == 1){
		fprintf(cgiOut, "<p %s><a href=\"%s?mode=update_ker\"> Kernel Update </a></p>\n",
				(KER_HTML==index)?on:"", SETTING);
	}
	fprintf(cgiOut, "<p %s><a href=\"%s?mode=reboot\"> Server Restart </a></p>\n",
			(REBOOT_HTML==index)?on:"", SETTING);
	fprintf(cgiOut, "</li>\n");
	fprintf(cgiOut, "</ul>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "</div>\n");

}

/**
 * *  @brief       print ethernet html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void printEthernetHtml(struct setting_info *setting)
{
	struct ethernet_info *info = &setting->ethernet;

	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, ETH_HTML);
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> Ethernet</div>\n");
	fprintf(cgiOut, "<div>\n");
	fprintf(cgiOut, "<form action=\"%s\" method=\"post\" name=\"networkForm\">\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" id=\"mode\" name=\"mode\" value=\"ethernet_save\" />\n");
	fprintf(cgiOut, "<input type=\"hidden\" name=\"ip\" id=\"ip\" value=\"%s\"/>\n", info->ipaddr);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"subnetMask\" id=\"subnetMask\" value=\"%s\"/>\n", info->netmask);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"gateWay\" id=\"gateWay\" value=\"%s\"/>\n", info->gateway);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"dnsServer1\" id=\"dnsServer1\" value=\"%s\"/>\n", info->firstdns);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"dnsServer2\" id=\"dnsServer2\" value=\"%s\"/>\n", info->backdns);
	fprintf(cgiOut, "<table class=\"tablist\" width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\">\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td width=\"30%%\" align=\"right\"> Automatically Obtain An IP Address:</td>\n");
	fprintf(cgiOut, "<td width=\"70%%\">\n");
	fprintf(cgiOut, "<input name=\"autoIp\" id=\"autoIp\" value=\"%s\" type=\"checkbox\" class=\"checkbox\" onclick=\"checkAutoIp(this)\" %s />\n",
			info->dhcp, !strcmp(info->dhcp, "1")?"checked":"");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> IP Address:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<script type=\"text/javascript\">\n");
	fprintf(cgiOut, "showNetworkConfigInput(\"ip\")\n");
	fprintf(cgiOut, "</script>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Subnet Mask:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<script type=\"text/javascript\">\n");
	fprintf(cgiOut, "showNetworkConfigInput(\"subnetMask\")\n");
	fprintf(cgiOut, "</script>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Default Gateway:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<script type=\"text/javascript\">\n");
	fprintf(cgiOut, "showNetworkConfigInput(\"gateWay\")\n");
	fprintf(cgiOut, "</script>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Preferred DNS Server:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<script type=\"text/javascript\">\n");
	fprintf(cgiOut, "showNetworkConfigInput(\"dnsServer1\")\n");
	fprintf(cgiOut, "</script>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Secondary DNS Server:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<script type=\"text/javascript\">\n");
	fprintf(cgiOut, "showNetworkConfigInput(\"dnsServer2\")\n");
	fprintf(cgiOut, "</script>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td></td>\n");
	fprintf(cgiOut, "<td height=\"50\" valign=\"middle\">\n");
	fprintf(cgiOut, "<input  class=\"btn\" type=\"button\" value=\"Save \"  onclick=\"submitNetworkConfig()\"/>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fprintf(cgiOut, "<script type=\"text/javascript\">\n");
	fprintf(cgiOut, "setNetworkConfig();\n");
	fprintf(cgiOut, "checkAutoIp(document.getElementById(\"autoIp\"));\n");
	fprintf(cgiOut, "</script>\n");
	fflush(cgiOut);
}

/**
 * *  @brief       search ap info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      error
 * */
static int doWirelessSearch(struct setting_info *setting)
{
	int error;

	cgiInitApInfo(&setting->wireless.ap_info);
	error = cgiScanConnectAp(&setting->wireless.ap_info);
	error |= cgiScanOtherAp(&setting->wireless.ap_info);
	return error;

}

/**
 * *  @brief       print wifi list to html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printWifiList(struct setting_info *setting)
{
	int index;
	struct ap_shop *info=&setting->wireless.ap_info;
	struct ap *con_ap=&info->ap_list[0];

	for (index=1; index <= info->ap_num; index++)
	{
		struct ap *ap;
		int mode=0;
		int level;
		char *sig[] = {
			" weaker",
			" weak",
			" middle",
			" strong",
		};

		ap = &info->ap_list[index];

		if (!strcmp(ap->mode, "Master")){
			mode = 0;
		}
		else{
			mode = 1;
		}

		level = -100;
		sscanf(ap->sig_level, "%d", &level);
		if (level < -86){
			level = 0;
		}
		else if (level < -71){
			level = 1;
		}
		else if (level < -56){
			level = 2;
		}
		else{
			level = 3;
		}

		fprintf(cgiOut, "<tr onclick=\"selectWifi(this)\"  align=\"center\" style=\"cursor:pointer\">\n");
		fprintf(cgiOut, "<td>%d</td>\n", index);
		fprintf(cgiOut, "<td>%s</td>\n", ap->ssid);
		fprintf(cgiOut, "<td>%s</td>\n", wifi_mode[mode]);
		fprintf(cgiOut, "<td>%s</td>\n", safe[ap->security]);
		fprintf(cgiOut, "<td>%s</td>\n", !strcmp(con_ap->ssid, ap->ssid)?" Connected ":" Unconnected ");
		fprintf(cgiOut, "<td>%s</td>\n", sig[level]);
		fprintf(cgiOut, "</tr>\n");
	}
}

/**
 * *  @brief       print wifi ap mode to html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printWirelessHtmlMode(struct setting_info *setting)
{
	int index = 0;

	for (; index < sizeof(wifi_mode)/sizeof(wifi_mode[0]); index++){
		fprintf(cgiOut, "<option value=\"%s\">%s</option>\n",
				wifi_mode[index], wifi_mode[index]);
	}
}
/**
 * *  @brief       print wifi ap safe to html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printWirelessHtmlSafe(struct setting_info *setting)
{
	int index = 0;

	for (; index < sizeof(safe)/sizeof(safe[0]); index++){
		fprintf(cgiOut, "<option value=\"%s\">%s</option>\n",
				safe[index], safe[index]);
	}
}

/**
 * *  @brief       print wifi html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printWirelessHtml(struct setting_info *setting)
{
	doWirelessSearch(setting);

	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, WIFI_HTML);

	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> Wireless Network </div>\n");
	fprintf(cgiOut, "<div id=\"selectWifiDiv\">\n");
	fprintf(cgiOut, "<form action=\"%s?mode=wireless\" method=\"post\" name=\"wifiForm\" onsubmit=\"return wifiConfigVerify()\">\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"mode\"  id=\"mode\" name=\"mode\" value=\"wireless_save\"/>\n");
	fprintf(cgiOut, "<table class=\"tablist\" width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\">\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Wireless Network SSID:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" name=\"ssid\" id=\"ssid\" readonly=\"readonly\"/>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Wireless Network Mode:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" name=\"wifiMode\" id=\"wifiMode\" readonly=\"readonly\" />\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Security:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" name=\"safe\" id=\"safe\" readonly=\"readonly\" />\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Password:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" name=\"password\" id=\"password\" />\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td></td>\n");
	fprintf(cgiOut, "<td height=\"50\" valign=\"middle\"><input name=\"input\" class=\"btn\" type=\"button\" value=\" Search \" onclick=\"window.location.href=document.location.href\"/>\n");
	fprintf(cgiOut, "&nbsp;&nbsp;&nbsp;&nbsp;   <input  class=\"btn\" type=\"button\" value=\" Add \" onclick=\"showAddWifi();\" />\n");
	fprintf(cgiOut, "&nbsp;&nbsp;&nbsp;&nbsp;   <input  class=\"btn\" type=\"submit\" value=\" Save \" />\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</form>\n");

	fprintf(cgiOut, "<div class=\"listdiv\">\n");
	fprintf(cgiOut, "<table width=\"100%%\" border=\"0\" class=\"wlist\" cellspacing=\"0\" cellpadding=\"0\">\n");
	fprintf(cgiOut, "<tr class=\"title\" >\n");
	fprintf(cgiOut, "<td> No.</td>\n");
	fprintf(cgiOut, "<td> Wireless Network SSID </td>\n");
	fprintf(cgiOut, "<td> Wireless Network Mode </td>\n");
	fprintf(cgiOut, "<td> Security </td>\n");
	fprintf(cgiOut, "<td> Connection status </td>\n");
	fprintf(cgiOut, "<td style=\"border-right:0 none;\"> Signal </td>\n");
	fprintf(cgiOut, "</tr>\n");
	printWifiList(setting);
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</div>\n");

	fprintf(cgiOut, "<div id=\"addWifiDiv\" style=\"display:none;\">\n");
	fprintf(cgiOut, "<form action=\"%s?mode=wireless\" method=\"post\" name=\"addWifiForm\" id=\"addWifiForm\" onsubmit=\"return addWifiConfigVerify()\">\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"mode\"  value=\"wireless_add_save\"/>\n");
	fprintf(cgiOut, "<table class=\"tablist\" width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\">\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Wireless Network SSID:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" name=\"ssid\"  />\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Wireless Network Mode:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<select name=\"wifiMode\" >\n");
	printWirelessHtmlMode(setting);
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Security:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<select name=\"safe\" onchange=\"changeWifiSafe(this.value)\">\n");
	printWirelessHtmlSafe(setting);
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Password:</td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" name=\"password\" disabled=\"true\"/>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td></td>\n");
	fprintf(cgiOut, "<td height=\"50\" valign=\"middle\"><input  class=\"btn\" type=\"submit\" value=\" Save \" />\n");
	fprintf(cgiOut, "&nbsp;&nbsp;&nbsp;&nbsp;   <input  class=\"btn\" type=\"button\" value=\" Cancel \" onclick=\"showSelectWifi();\" />\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");

	fflush(cgiOut);
}

/**
 * *  @brief       print wifi softap  html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printSoftapHtml(struct setting_info *setting)
{
	struct softap_info *softap = &setting->softap;

	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, SAP_HTML);
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> SoftAP Setup </div>\n");
	fprintf(cgiOut, "<form action=\"%s\" method=\"post\" name=\"updateForm\"  onsubmit=\"return softAPCheckUp()\">\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"mode\"  id=\"mode\" name=\"mode\" value=\"softap_save\"/>\n");
	fprintf(cgiOut, "<center style=\"padding:100px;\">\n");
	fprintf(cgiOut, "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\" >\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " Wireless Network SSID:\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"%s\" />\n", softap->ssid);
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " Password:\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<input type=\"password\" id=\"psd\" name=\"psd\"/>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " Confirm Password:\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<input type=\"password\" id=\"confirmPassword\" name=\"confirmPassword\"/>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " Security:\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<span id=\"safeText\" name=\"safeText\">WPA/WPA2 PSK</span>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</center>\n");
	fprintf(cgiOut, "<div align=\"center\"><input  class=\"btn\" type=\"submit\" value=\"Save\" /></div>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
}


/**
 * *  @brief       print video  html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printVideoHtml(struct setting_info *setting)
{
	struct video_info *info = &setting->video;

	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, VID_HTML);
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> Video </div>\n");
	fprintf(cgiOut, "<div>\n");
	fprintf(cgiOut, "<form action=\"%s\" method=\"post\" name=\"videoForm\" onsubmit=\"return videoConfigVerify()\">\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"mode\"  id=\"mode\" name=\"mode\" value=\"video_save\"/>\n");
	//fprintf(cgiOut, "<div  style=\"padding:10px 0px 10px 50px;font-size:16px\">\n");
	//fprintf(cgiOut, "<input type=\"radio\" name=\"radioKbps\" id=\"radioKbpsOne\" class=\"checkbox\" value=\"0\" %s onclick=\"autoKbps('0')\"/> Stream Setup Manually \n", !strcmp(info->auto_kbps, "0")?"checked":"");
	//fprintf(cgiOut, "<input type=\"radio\" name=\"radioKbps\" id=\"radioKbpsTwo\" class=\"checkbox\" value=\"1\" %s onclick=\"autoKbps('1')\"/> Stream Adjust Automatically \n", !strcmp(info->auto_kbps, "1")?"checked":"");
	//fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<div id=\"allKpbs\">\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<div class=\"timerbox\">\n");
	fprintf(cgiOut, "<span class=\"timerspan\"> Stream 1</span>\n");
	fprintf(cgiOut, "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\" >\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td width=\"16%%\"> Coding Format </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<select id=\"codedFormatOne\" name=\"codedFormatOne\" onchange=\"codedFormat(this.value)\">\n");
	fprintf(cgiOut, "<option value=\"h264\" %s>H.264</option>\n",
			!strcmp(info->video1.format,"h264")?"selected":"");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td width=\"16%%\"> Resolution </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<select id=\"DPIOne\" name=\"DPIOne\">\n");
	fprintf(cgiOut, "<option value=\"720\" %s>720P(1280x720)</option>\n",
			!strcmp(info->video1.dpi,"720")?"selected":"");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td width=\"16%%\" align=\"right\"> Rate </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" name=\"staticKpbs\" id=\"staticKpbs\" value=\"%s\"/>Kbps(256-6144)\n", info->video1.kbps);
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td width=\"16%%\" align=\"right\"> Rate Mode </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"radio\" name=\"kbpsOne\" id=\"staticRadioKpbsOne\" value=\"static_kbps\" class=\"checkbox\" %s/>Constant Rate \n", !strcmp(info->video1.kbps_mode, "static_kbps")?"checked":"");
	fprintf(cgiOut, "<input type=\"radio\" name=\"kbpsOne\" id=\"runRadioKpbsOne\" value=\"dynamic_kbps\" class=\"checkbox\" %s/>Dynamic Rate \n", !strcmp(info->video1.kbps_mode, "dynamic_kbps")?"checked":"");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td width=\"16%%\" align=\"right\"> Frame rate </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" id=\"frameRateOne\" name=\"frameRateOne\" value=\"%s\" style=\"width:40px\"/>fps(1-30)\n", info->video1.fps);
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td width=\"16%%\" align=\"right\"> I Frame Interval </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" id=\"iFrameRateOne\" name=\"iFrameRateOne\" value=\"%s\" style=\"width:40px\"/>(2-150)\n", info->video1.group);
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<div class=\"timerbox\">\n");
	fprintf(cgiOut, "<span class=\"timerspan\"> Stream 2</span>\n");
	fprintf(cgiOut, "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\" >\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Coding Format </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<select id=\"codedFormatTwo\" name=\"codedFormatTwo\" onchange=\"codedFormat2(this.value)\">\n");
	fprintf(cgiOut, "<option value=\"h264\" %s>H.264</option>\n",
			!strcmp(info->video2.format,"h264")?"selected":"");
	fprintf(cgiOut, "<option value=\"mjpeg\" %s>MJPEG</option>\n",
			!strcmp(info->video2.format,"mjpeg")?"selected":"");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Resolution </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<select id=\"DPITwo\" name=\"DPITwo\">\n");
	fprintf(cgiOut, "<option valye=\"d1\" %s>D1(720*576)</option>\n",
			!strcmp(info->video2.dpi,"d1")?"selected":"");
	fprintf(cgiOut, "<option value=\"vga\" %s>VGA(640*480)</option>\n",
			!strcmp(info->video2.dpi,"vga")?"selected":"");
	fprintf(cgiOut, "<option value=\"qvga\" %s>QVGA(320*240)</option>\n",
			!strcmp(info->video2.dpi,"qvga")?"selected":"");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Rate </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" value=\"%s\" name=\"staticKpbs2\" id=\"staticKpbs2\"/>Kbps(64-6144)\n", info->video2.kbps);
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr id=\"picQualityTr\">\n");
	fprintf(cgiOut, "<td align=\"right\"> Image Quality </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<select id=\"picQuality\" name=\"picQuality\" style=\"width:50px\">\n");
	fprintf(cgiOut, "<option value=\"0\" %s>0</option>\n", !strcmp(info->video2.quality,"0")?"selected":"");
	fprintf(cgiOut, "<option value=\"1\" %s>1</option>\n", !strcmp(info->video2.quality,"1")?"selected":"");
	fprintf(cgiOut, "<option value=\"2\" %s>2</option>\n", !strcmp(info->video2.quality,"2")?"selected":"");
	fprintf(cgiOut, "<option value=\"3\" %s>3</option>\n", !strcmp(info->video2.quality,"3")?"selected":"");
	fprintf(cgiOut, "<option value=\"4\" %s>4</option>\n", !strcmp(info->video2.quality,"4")?"selected":"");
	fprintf(cgiOut, "<option value=\"5\" %s>5</option>\n", !strcmp(info->video2.quality,"5")?"selected":"");
	fprintf(cgiOut, "<option value=\"6\" %s>6</option>\n", !strcmp(info->video2.quality,"6")?"selected":"");
	fprintf(cgiOut, "<option value=\"7\" %s>7</option>\n", !strcmp(info->video2.quality,"7")?"selected":"");
	fprintf(cgiOut, "<option value=\"8\" %s>8</option>\n", !strcmp(info->video2.quality,"8")?"selected":"");
	fprintf(cgiOut, "<option value=\"9\" %s>9</option>\n", !strcmp(info->video2.quality,"9")?"selected":"");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr id=\"kbpsTrTwo\">\n");
	fprintf(cgiOut, "<td align=\"right\"> Rate Mode </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"radio\" name=\"kbpsTwo\" id=\"staticRadioKpbsTwo\" value=\"static_kbps\" class=\"checkbox\" %s/>Constant Rate \n", !strcmp(info->video2.kbps_mode, "static_kbps")?"checked":"");
	fprintf(cgiOut, "<input type=\"radio\" name=\"kbpsTwo\" id=\"runRadioKpbsTwo\" value=\"dynamic_kbps\" class=\"checkbox\" %s/>Dynamic Rate \n", !strcmp(info->video2.kbps_mode, "dynamic_kbps")?"checked":"");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\"> Frame rate </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" id=\"frameRateTwo\" name=\"frameRateTwo\" value=\"%s\" style=\"width:40px\"/>fps(1-30)\n", info->video2.fps);
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr id=\"iFrameRateTrTwo\">\n");
	fprintf(cgiOut, "<td align=\"right\"> I Frame Interval </td>\n");
	fprintf(cgiOut, "<td>\n");
	fprintf(cgiOut, "<input type=\"text\" id=\"iFrameRateTwo\" name=\"iFrameRateTwo\" value=\"%s\" style=\"width:40px\"/>(2-150)\n", info->video2.group);
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<div  align=\"center\"><input  class=\"btn\" type=\"submit\" value=\"Save \" /></div>\n");
	fprintf(cgiOut, "</br>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<script type=\"text/javascript\">\n");
	fprintf(cgiOut, "videoConfigInit();\n");
	fprintf(cgiOut, "</script>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
}


/**
 * *  @brief       print picture setting html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printPictureHtml(struct setting_info *setting)
{
	struct picture_info *info = &setting->picture;

	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, PIC_HTML);
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> Image </div>\n");
	fprintf(cgiOut, "<form action=\"%s\" method=\"post\" name=\"resetForm\">\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"mode\"  id=\"mode\" name=\"mode\" value=\"picture_save\"/>\n");
	fprintf(cgiOut, "<center  style=\"padding:100px;\">\n");
	fprintf(cgiOut, "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\" >\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " Photo stream \n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<select id=\"picKbps\" name=\"picKbps\">\n");
	fprintf(cgiOut, "<option value=\"1\" %s> Stream 1</option>\n",
			!strcmp(info->video_index, "1")?"selected":"");
	fprintf(cgiOut, "<option value=\"2\" %s> Stream 2</option>\n",
			!strcmp(info->video_index, "2")?"selected":"");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " Lighting Power Frequency \n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<select id=\"ELHz\" name=\"ELHz\">\n");
	fprintf(cgiOut, "<option value=\"50\" %s>50HZ</option>\n",
			!strcmp(info->el_hz, "50")?"selected":"");
	fprintf(cgiOut, "<option value=\"60\" %s>60HZ</option>\n",
			!strcmp(info->el_hz, "60")?"selected":"");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " OSD Name \n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<input type=\"text\" id=\"OSDName\" name=\"OSDName\" value=\"%s\" maxlength=\"10\"/>\n", info->osd_name);
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " OSD Date and Time \n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<select id=\"OSDTime\" name=\"OSDTime\">\n");
	fprintf(cgiOut, "<option value=\"hide\" %s> Hide </option>\n",
			!strcmp(info->osd_time, "hide")?"selected":"");
	fprintf(cgiOut, "<option value=\"show\" %s> Show </option>\n",
			!strcmp(info->osd_time, "show")?"selected":"");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " OSD Position \n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<select id=\"OSDPlace\" name=\"OSDPlace\">\n");
	fprintf(cgiOut, "<option value=\"left_up\" %s> Left Up </option>\n",
			!strcmp(info->osd_place, "left_up")?"selected":"");
	fprintf(cgiOut, "<option value=\"right_up\" %s> Right Up </option>\n",
                        !strcmp(info->osd_place, "right_up")?"selected":"");
	fprintf(cgiOut, "<option value=\"left_down\" %s> Left Down </option>\n",
                        !strcmp(info->osd_place, "left_down")?"selected":"");
	fprintf(cgiOut, "<option value=\"right_down\" %s> Right Down </option>\n",
                        !strcmp(info->osd_place, "right_down")?"selected":"");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</center>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<br/>\n");
	fprintf(cgiOut, "<br/>\n");
	fprintf(cgiOut, "<div  align=\"center\"><input  class=\"btn\" type=\"submit\" value=\"Save \" /></div>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
}


/**
 * *  @brief       print recoder setting html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printRecoderHtml( struct setting_info *setting)
{
	struct recoder_info *info = &setting->recoder;

	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, REC_HTML);
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> Recorder </div>\n");
	fprintf(cgiOut, "<form action=\"%s\" method=\"post\" name=\"videoForm\" >\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"mode\"  id=\"mode\" name=\"mode\" value=\"recoder_save\"/>\n");
	fprintf(cgiOut, "<center style=\"padding:100px;\">\n");
	fprintf(cgiOut, "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\" >\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " Video Stream \n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<select id=\"recoderKbps\" name=\"recoderKbps\" style=\"width:150px\">\n");
	fprintf(cgiOut, "<option value=\"1\" %s> Stream 1</option>\n", !strcmp(info->video_index, "1")?"selected":"");
	fprintf(cgiOut, "<option value=\"2\" %s> Stream 2</option>\n", !strcmp(info->video_index, "2")?"selected":"");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " Loop Recording Time \n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<select id=\"recoderTime\" name=\"recoderTime\">\n");
	fprintf(cgiOut, "<option value=\"2\" %s>2 Minutes </option>\n",
			!strcmp(info->length, "2")?"selected":"");
	fprintf(cgiOut, "<option value=\"5\" %s>5 Minutes </option>\n",
			!strcmp(info->length, "5")?"selected":"");
	fprintf(cgiOut, "<option value=\"15\" %s>15 Minutes </option>\n",
			!strcmp(info->length, "15")?"selected":"");
	fprintf(cgiOut, "<option value=\"30\" %s>30 Minutes </option>\n",
			!strcmp(info->length, "30")?"selected":"");
	fprintf(cgiOut, "</select>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\">\n");
	fprintf(cgiOut, " Timed Loop Recording \n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td align=\"left\">\n");
	fprintf(cgiOut, "<input type=\"text\" id=\"recoderOnTime\" name=\"recoderOnTime\" style=\"width:40px;\" value=\"%s\"/> Hours \n", info->time);
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</center>\n");
	fprintf(cgiOut, "<div  align=\"center\"><input  class=\"btn\" type=\"submit\" value=\" Save \" /></div>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
}

/**
 * *  @brief       print user setting html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printUserHtml(struct setting_info *setting)
{
	struct user_info *info = &setting->user;

	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, USER_HTML);
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> User Control </div>\n");
	fprintf(cgiOut, "<div>\n");
	fprintf(cgiOut, "<form action=\"%s\" method=\"post\" name=\"userForm\" onsubmit=\"return userVerify()\">\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" id=\"mode\" name=\"mode\" value=\"user_save\" />\n");
	fprintf(cgiOut, "<table class=\"tablist\" width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\"  >\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td width=\"30%%\" align=\"right\"> User Name:</td>\n");
	fprintf(cgiOut, "<td width=\"70%%\"><input type=\"text\" name=\"userName\" id=\"userName\" value=\"%s\"/></td>\n", info->name);
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\" valign=\"top\"> Password: </td>\n");
	fprintf(cgiOut, "<td valign=\"top\"><input type=\"password\" name=\"password\" id=\"password\" /></td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\" valign=\"top\"> Confirm Password: </td>\n");
	fprintf(cgiOut, "<td valign=\"top\"><input type=\"password\" name=\"password2\" id=\"password2\" /></td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\" valign=\"top\">&nbsp;</td>\n");
	fprintf(cgiOut, "<td valign=\"top\"><input name=\"input\" class=\"btn\" type=\"submit\" value=\" Save \" /></td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
}

static void printRecoverHtml(struct setting_info *setting)
{
	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, REV_HTML);
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> System Configuration </div>\n");
	fprintf(cgiOut, "<form action=\"%s\" method=\"post\" name=\"resetForm\" onsubmit=\"return confirmReset()\">\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" id=\"mode\" name=\"mode\" value=\"recover_save\" />\n");
	fprintf(cgiOut, "<center  style=\"padding:100px;\">\n");
	fprintf(cgiOut, "<input  class=\"btn2\" type=\"submit\" value=\" Factory Reset \" />\n");
	fprintf(cgiOut, "</center>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
}


static void printSystemHtml(struct setting_info *setting)
{
#if 0
	struct system_info *info = &setting->system;

	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, SYS_HTML);
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> System Configuration </div>\n");
	fprintf(cgiOut, "<div>\n");
	fprintf(cgiOut, "<form  action=\"\" method=\"post\" name=\"systemInfoForm\" onsubmit=\"return sysConfigVerify()\">\n");
	fprintf(cgiOut, "<input type=\"hidden\" id=\"mode\" name=\"mode\" value=\"system_save\" />\n");
	fprintf(cgiOut, "<table class=\"tablist\" width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\"  >\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td width=\"30%%\" align=\"right\">SN:</td>\n");
	fprintf(cgiOut, "<td width=\"70%%\"><input type=\"text\" name=\"sn\" id=\"sn\" value=\"%s\"  readonly=\"readonly\"/></td>\n", info->sn);
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\" valign=\"top\"> Access Password: </td>\n");
	fprintf(cgiOut, "<td valign=\"top\"><input type=\"text\" name=\"password\" id=\"password\" /></td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\" valign=\"top\"> Manufacturer: </td>\n");
	fprintf(cgiOut, "<td valign=\"top\"><input type=\"text\" name=\"manufacturers\" id=\"manufacturers\"  value=\"%s\" readonly=\"readonly\"/></td>\n", info->product);
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\" valign=\"top\"> Model:</td>\n");
	fprintf(cgiOut, "<td valign=\"top\"><input type=\"text\" name=\"model\" id=\"model\" value=\"%s\" readonly=\"readonly\"/></td>\n", info->type);
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td colspan=\"2\" align=\"right\" valign=\"top\">\n");
	fprintf(cgiOut, "<div  align=\"center\">\n");
	fprintf(cgiOut, "<input  class=\"btn\" type=\"submit\" value=\" Save \" />\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
#endif
}

static void printUpdateHtml(struct setting_info *setting)
{

#if 0
	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, UPD_HTML);
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> System Configuration </div>\n");
	fprintf(cgiOut, "<form action=\"%s\" method=\"post\" name=\"resetForm\" onsubmit=\"return confirmReset()\">\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" id=\"mode\" name=\"mode\" value=\"update_save\" />\n");
	fprintf(cgiOut, "<center  style=\"padding:100px;\">\n");
	fprintf(cgiOut, "<input  class=\"btn2\" type=\"submit\" value=\" System Upgrade \" />\n");
	fprintf(cgiOut, "</center>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
#endif
}


/**
 * *  @brief       print update app setting html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printUpdateAppHtml(struct setting_info *setting, char *alert)
{
	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, APP_HTML);
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> System Configuration </div>\n");
	fprintf(cgiOut, "<form action=\"%s\" method=\"post\" name=\"updateForm\" onsubmit=\"return appCheckUp()\">\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"mode\"  id=\"mode\" name=\"mode\" value=\"upapp_save\"/>\n");
	fprintf(cgiOut, "<center  style=\"padding:100px;\">\n");
	fprintf(cgiOut, "<span style=\"font-size:16px;font-weight:bold\"> Application Upgrades Address:</span><input type=\"text\" id=\"updateUrl\" name=\"updateUrl\" style=\"width:260px;height:20px\"/>&nbsp;<input  class=\"btn\" type=\"submit\" value=\" Save \" />\n");

	fprintf(cgiOut, "</center>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "%s\n", alert);
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
}

/**
 * *  @brief       do and print update app setting html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void doAndPrintUpdateAppHtml(struct setting_info *setting)
{
	char string[256];
	cgiFormResultType ret;
	char *fail[] = {
		"<script type=\"text/javascript\">alert(\"SD card is not inserted!\");</script>",
		"<script type=\"text/javascript\">alert(\"Download Failed!\");</script>",
		"<script type=\"text/javascript\">alert(\"File error!\");</script>",
		"<script type=\"text/javascript\">alert(\"The download is complete, do you want to upgrade?\");document.location.href=\'setting?mode=update_app\';</script>",
	};

	ret = cgiFormStringNoNewlines("updateUrl", string, sizeof(string));
	cgiHtmlEscape(string);
	settingDebug("url: %s\n", string);

	//test
	if (!strcmp(string, "0")){
		printUpdateAppHtml(setting, fail[0]);
	}
	else if (!strcmp(string, "1")){
		printUpdateAppHtml(setting, fail[1]);
	}
	else if (!strcmp(string, "2")){
		printUpdateAppHtml(setting, fail[2]);
	}
	else if (!strcmp(string, "3")){
		printUpdateAppHtml(setting, fail[3]);
	}
	else{
		printUpdateAppHtml(setting, "");
	}
	//test end
}


/**
 * *  @brief       print update kernel setting html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printUpdateKerHtml(struct setting_info *setting, char *alert, int can_update)
{
	int nNoUpdate = 0;

	char *fail[] = {
		"<script type=\"text/javascript\">alert(\"SD card is not inserted!\");</script>",
		"<script type=\"text/javascript\">alert(\"Download Failed!\");</script>",
		"<script type=\"text/javascript\">alert(\"File error!\");</script>",
		"<script type=\"text/javascript\">if(confirm(\"The download is complete, do you want to upgrade?\")){document.location.href=\'setting?mode=update_ker_burn&n=yes\';}else{document.location.href=\'setting?mode=update_ker_burn&n=no\';}</script>",
	};

	if (semctl(sem_id, KERNEL_UPDATE_SD, GETVAL) == 0) {
		alert = fail[0];
		semctl(sem_id, KERNEL_UPDATE_SD, SETVAL, 1);
	}else if (semctl(sem_id, KERNEL_UPDATE_DOWNLOAD, GETVAL) == 0) {
		alert = fail[1];
		semctl(sem_id, KERNEL_UPDATE_DOWNLOAD, SETVAL, 1);
	}else if (semctl(sem_id, KERNEL_UPDATE_CAN_BURN_KER, GETVAL) == 1){
		alert = fail[3];
		semctl(sem_id, KERNEL_UPDATE_CAN_BURN_KER, SETVAL, 0);
	}else if (semctl(sem_id, KERNEL_UPDATE_FILE, GETVAL) == 0) {
		alert = fail[2];
		semctl(sem_id, KERNEL_UPDATE_FILE, SETVAL, 1);
	}else if (strlen(alert) == 0){
		 alert = "";
	}else {
	}

	cgiHeaderContentType("text/html");
	printLeftIndexHtml(setting, KER_HTML);
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> Kernel Upgrade </div>\n");
	fprintf(cgiOut, "<form action=\"%s?mode=update_ker\" method=\"post\" name=\"rootForm\" onsubmit=\"return rootCheckUp()\">\n", SETTING);
	fprintf(cgiOut, "<input type=\"hidden\" name=\"mode\"  id=\"mode\" name=\"mode\" value=\"upker_save\"/>\n");
	fprintf(cgiOut, "<center  style=\"padding:100px;\">\n");
	nNoUpdate = semctl(sem_id, KERNEL_UPDATE_OP, GETVAL);
	if (can_update && nNoUpdate)
		fprintf(cgiOut, "<span style=\"font-size:16px;font-weight:bold\"> Kernel Upgrade Address:</span><input type=\"text\" id=\"rootUrl\" name=\"rootUrl\" style=\"width:260px;height:20px\"/>&nbsp;<input  class=\"btn\" type=\"submit\" value=\" OK \" />\n");
	else
		fprintf(cgiOut, "<span style=\"font-size:16px;font-weight:bold\">Update...! please wait...</span>\n");
	fprintf(cgiOut, "</center>\n");
  fprintf(cgiOut, "<div align=\"center\">\n");
  /*
  fprintf(cgiOut, "<font color=\"red\" size=\"4\">%s</font>", alert);
  fprintf(cgiOut, "</div>");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	*/
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "%s\n", alert);
	if (!(can_update && nNoUpdate))
	{
		fprintf(cgiOut, "<script type=\"text/javascript\">\n");
		fprintf(cgiOut, "setTimeout(\"refreshPage();\", 5000);");
		fprintf(cgiOut, "</script>");
	}
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
}

static void printRebootHtml(struct setting_info *setting)
{
        cgiHeaderContentType("text/html");
        printLeftIndexHtml(setting, REBOOT_HTML);
        fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
        fprintf(cgiOut, "<div class=\"title\"> Server Restart </div>\n");
        fprintf(cgiOut, "<form action=\"%s\" method=\"post\" name=\"rebootForm\" onsubmit=\"return confirmReboot()\">\n", SETTING);
        fprintf(cgiOut, "<input type=\"hidden\" id=\"mode\" name=\"mode\" value=\"reboot_save\" />\n");
        fprintf(cgiOut, "<center  style=\"padding:100px;\">\n");
        fprintf(cgiOut, "<input  class=\"btn2\" type=\"submit\" value=\" Restart \" />\n");
        fprintf(cgiOut, "</center>\n");
        fprintf(cgiOut, "</form>\n");
        fprintf(cgiOut, "</div>\n");
        fprintf(cgiOut, "<div class=\"clear\"></div>\n");
        fprintf(cgiOut, "</div>\n");
        fprintf(cgiOut, "</body>\n");
        fprintf(cgiOut, "</html>\n");
        fflush(cgiOut);
}

/**
 * *  @brief       system V semaphore pv operation
 * *  @author
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static int semaphore_pv(int sem_id, int sem_index, int op)
{
	struct sembuf sbuf;
	sbuf.sem_num = sem_index;
	sbuf.sem_op = op;
	sbuf.sem_flg = SEM_UNDO;

	if (semop(sem_id, &sbuf, 1) < 0) {
		print_error("%s->%s:semop fail, error %d!", __FILE__, __func__, errno);
		return -1;
	}

	return 0;
}

/**
 * *  @brief       do and print update kernel setting html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void doAndPrintUpdateKerHtml(struct setting_info *setting)
{
	char string[256];
	//cgiFormResultType ret;

	char *fail[] = {
		"<script type=\"text/javascript\">alert(\"SD card is not inserted!\");</script>",
		"<script type=\"text/javascript\">alert(\"Download Failed!\");</script>",
		"<script type=\"text/javascript\">alert(\"File error!\");</script>",
		"<script type=\"text/javascript\">alert(\"The download is complete, do you want to upgrade?\");document.location.href=\'setting?mode=update_ker\';</script>",
	};

	//ret = cgiFormStringNoNewlines("rootUrl", string, sizeof(string));
	//settingDebug("%s:ker update address %s\n", __func__, string);

	/*char cmd[300];
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "httpclient %s", string);
	system(cmd);
	*/

	//cgiHtmlEscape(string);
	//settingDebug("%s:ker update address escape %s\n", __func__, string);

	if (!strcmp(string, "0")){
		printUpdateKerHtml(setting, fail[0], 0);
	}
	else if (!strcmp(string, "1")){
		printUpdateKerHtml(setting, fail[1], 0);
	}
	else if (!strcmp(string, "2")){
		printUpdateKerHtml(setting, fail[2], 0);
	}
	else if (!strcmp(string, "3")){
		printUpdateKerHtml(setting, fail[3], 0);
	}
	else{
		printUpdateKerHtml(setting, "", 0);
	}
}

/**
 * *  @brief       update reboot html
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting info
 * *  @return      none
 * */
static void UpdateRebootInform(struct setting_info * setting)
{
	printRebootHtml(setting);
}

/**
 * *  @brief       burn kernel to the spi flash
 * *  @author
 * *  @date        2013-5-9
 * *  @param[in]   setting info
 * *  @return      none
 * */
static void burnAndPrintfUpdateKerHtml(struct setting_info * setting)
{
	char string[LEN64];
	int ret = 0;

	ret = cgiFormStringNoNewlines("n", string, sizeof(string));
	settingDebug("%s:ker update %s\n", __func__, string);

	if ((semctl(sem_id, KERNEL_UPDATE_OP_DOWNLOAD, GETVAL) == 0) &&
			(strcmp(string, "yes") == 0)) {
		semaphore_pv(sem_id, KERNEL_UPDATE_OP_DOWNLOAD, 1);
		settingDebug("%s:ker update send the semaphore to httpclient, let it can burn the kernel\n", __func__);
	}else {
		semaphore_pv(sem_id, KERNEL_UPDATE_OP_DOWNLOAD, 2);
		settingDebug("%s:ker update send the semaphore to httpclient, let it can return\n", __func__);
	}

	printUpdateKerHtml(setting,
		"<script type=\"text/javascript\">document.location.href=\'setting?mode=update_ker\';</script>", 1);
}


/**
 * *  @brief       set browser cookie
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   cookie name, cookie value
 * *  @return	   none
 * */
static void cgiCookieSet(char *name, char *value)
{
	if (strlen(name))
	{
		fprintf(cgiOut, "Set-Cookie: %s=%s\r\n",	name, value);
	}
}



/**
 * *  @brief       print login html page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static void printLoginHtml(struct setting_info *setting)
{
	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
	fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf(cgiOut, "<head>\n");
	fprintf(cgiOut, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n");
	fprintf(cgiOut, "<title>%s</title>\n", setting->global.title);
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "<link href=\"../css/style.css\" rel=\"stylesheet\" type=\"text/css\" />\n");
	fprintf(cgiOut, "<script type=\"text/javascript\" src=\"../js/jquery.js\" ></script>\n");
	fprintf(cgiOut, "<script type=\"text/javascript\" src=\"../js/js.js\" ></script>\n");
	fprintf(cgiOut, "</head>\n");
	fprintf(cgiOut, "<body>\n");
	fprintf(cgiOut, "<div class=\"main\">\n");
	fprintf(cgiOut, "<div class=\"fleft  main_left\">\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\" main_right fleft\">\n");
	fprintf(cgiOut, "<div class=\"title\"> User Login </div>\n");
	fprintf(cgiOut, "<div>\n");
	fprintf(cgiOut, "<form action=\"\" method=\"post\" name=\"loginForm\" onsubmit=\"return loginVerify()\">\n");
	fprintf(cgiOut, "<input type=\"hidden\" id=\"mode\" name=\"mode\" value=\"user_login\" />\n");
	fprintf(cgiOut, "<table class=\"tablist\" width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"5\"  >\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td width=\"30%%\" align=\"right\"> User Name </td>\n");
	fprintf(cgiOut, "<td width=\"70%%\"><input type=\"text\" name=\"userName\" id=\"userName\" value=\"\"/></td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\" valign=\"top\"> Password </td>\n");
	fprintf(cgiOut, "<td valign=\"top\"><input type=\"password\" name=\"password\" id=\"password\" /></td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td align=\"right\" valign=\"top\">&nbsp;</td>\n");
	fprintf(cgiOut, "<td valign=\"top\">\n");
	fprintf(cgiOut, "<input class=\"btn\" type=\"submit\" value=\" Login \" />\n");
	fprintf(cgiOut, "<input class=\"btn\" type=\"reset\" value=\" Cancel\" />\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "<div class=\"clear\"></div>\n");
	fprintf(cgiOut, "</div>\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
}

/**
 * *  @brief       login
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      zero on already login
 * */
static int cgiLogin(struct setting_info *setting)
{
	char cvalue[LINE];
	char name[LEN32];
	char passwd[LEN32];
	int ret = 0;
	struct user_info *info = &setting->user;

	cgiCookieString("user", cvalue, sizeof(cvalue));

	if (strcmp(cvalue, "admin") == 0){
		setting->admin = 1;
	}
	else if (strcmp(cvalue, "guest") == 0){
		setting->admin = 0;
	}
	else{
		cgiFormString("userName", name, sizeof(name));
		cgiFormString("password", passwd, sizeof(passwd));
		if ((strcmp(name,info->name) == 0) && (strcmp(passwd,info->passwd) == 0)){
			cgiCookieSet("user", "guest");
			cgiRedirect(SETTING);
		}
		else if ((strcmp(name,info->root_name) == 0) && (strcmp(passwd,info->root_passwd) == 0)){
			cgiCookieSet("user", "admin");
			cgiRedirect(SETTING);
		}
		else{
			cgiCookieSet("user", "none");
			setting->admin = 0;
			printLoginHtml(setting);
		}
			ret = 1;
	}

	return ret;
}
static void doEthernet(struct setting_info *setting)
{
}

static void doWireless(struct setting_info *setting)
{

#if 0

	struct wireless_info *info = &setting->wireless;
	char cmd[LEN96];

	if (0 == strcmp(info->mode, wifi_mode[0]))
	{
		if (0 == strcmp(info->security, safe[0])){
			// share AP
			sprintf(cmd, "%s open %s > %s",
					WIFI_CONNET, info->ssid, SRCOUT);
			settingDebug("\n%s: %s\n", __func__, cmd);
			system(cmd);
		}
		else if(0 == strcmp(info->security, safe[1])){
			// WEP AP
			sprintf(cmd, "%s wep %s %s %d > %s",
					WIFI_CONNET, info->ssid, info->password, 0, SRCOUT);
			settingDebug("\n%s: %s\n", __func__, cmd);
			system(cmd);
		}
		else if(0 == strcmp(info->security, safe[2])){
			// WPA AP
			sprintf(cmd, "%s wpa %s %s > %s",
					WIFI_CONNET, info->ssid, info->password, SRCOUT);
			settingDebug("\n%s: %s\n", __func__, cmd);
			system(cmd);
		}
		else{
			// Error security
			print_error("%s->%s: Error security value!\n",
					__FILE__, __func__);
		}
	}
	else if (0 == strcmp(info->mode, wifi_mode[1]))
	{
		sprintf(cmd, "%s adhoc %s > %s",
				WIFI_CONNET, info->ssid, SRCOUT);
		settingDebug("\n%s: %s\n", __func__, cmd);
		system(cmd);
	}
	else{
		// Error mode
		print_error("%s->%s: Error mode value!\n",
				__FILE__, __func__);
	}
#endif

}

static void doWirelessAdd(struct setting_info *setting)
{

}

static void doSoftap(struct setting_info *setting)
{
}

static void doVideo(struct setting_info *setting)
{
}

static void doPicture(struct setting_info *setting)
{
}

static void doRecoder(struct setting_info *setting)
{
}

static void doFormatDisk(struct setting_info *setting)
{
}

static void doUser(struct setting_info *setting)
{
}

static void doRecover(struct setting_info *setting)
{
	system(SYSTEM_RECOVER);
}

static void doSystem(struct setting_info *setting)
{

}

static void doUpdateApp(struct setting_info *setting)
{
}

static void init_status_sem()
{
	int i = 0;

	for (i = KERNEL_UPDATE_SD; i <= KERNEL_UPDATE_FILE; ++i) {
		if (i == KERNEL_UPDATE_CAN_BURN_KER) semctl(sem_id, i, SETVAL, 0);
		else	semctl(sem_id, i, SETVAL, 1);
	}
}

static void doUpdateKer(struct setting_info *setting)
{
	//test
	int ret = 0;
	char string[256];

	if (semctl(sem_id, KERNEL_UPDATE_OP, GETVAL) > 0) {
		semaphore_pv(sem_id, KERNEL_UPDATE_OP, -1);

		init_status_sem();

		ret = cgiFormStringNoNewlines("rootUrl", string, sizeof(string));
		settingDebug("%s:ker update address %s\n", __func__, string);

		char cmd[300];
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "httpclient %s", string);
		system(cmd);

		semaphore_pv(sem_id, KERNEL_UPDATE_OP, 1);
	}
	//test end
}

static void doReboot(struct setting_info *setting)
{
	system(IPC_RESTART);
}

/**
 * *  @brief       save ethernet setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doEthernetSave(struct setting_info *setting)
{
	char string[LEN32];
	cgiFormResultType ret;

	cgiDebug("%s: ", __func__);
	if (cgiFormCheckboxSingle("autoIp") == cgiFormSuccess) {
		iniparser_set(setting->ini, "ethernet:dhcp", "1");
	}
	else{
		iniparser_set(setting->ini, "ethernet:dhcp", "0");

		ret = cgiFormString("ip", string, sizeof(string));
		iniparser_set(setting->ini, "ethernet:ipaddr", string);

		ret = cgiFormString("subnetMask", string, sizeof(string));
		iniparser_set(setting->ini, "ethernet:netmask", string);

		ret = cgiFormString("gateWay", string, sizeof(string));
		iniparser_set(setting->ini, "ethernet:gateway", string);

		ret = cgiFormString("dnsServer1", string, sizeof(string));
		iniparser_set(setting->ini, "ethernet:firstdns", string);

		ret = cgiFormString("dnsServer2", string, sizeof(string));
		iniparser_set(setting->ini, "ethernet:backdns", string);
	}

	storeSettingInfo(setting);
}

/**
 * *  @brief       save wireless setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doWirelessSave(struct setting_info *setting)
{
	char string[LEN32];
	cgiFormResultType ret;

	ret = cgiFormString("ssid", string, sizeof(string));
	iniparser_set(setting->ini, "wireless:ssid", string);

	ret = cgiFormString("wifiMode", string, sizeof(string));
	iniparser_set(setting->ini, "wireless:mode", string);

	ret = cgiFormString("safe", string, sizeof(string));
	iniparser_set(setting->ini, "wireless:security", string);

	ret = cgiFormString("password", string, sizeof(string));
	iniparser_set(setting->ini, "wireless:password", string);

	storeSettingInfo(setting);
}

/**
 * *  @brief       save wireless add setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doWirelessAddSave(struct setting_info *setting)
{
	char string[LEN32];
	int choice;
	cgiFormResultType ret;

	ret = cgiFormString("ssid", string, sizeof(string));
	iniparser_set(setting->ini, "wireless:ssid", string);

	ret = cgiFormSelectSingle("wifiMode", wifi_mode, sizeof(wifi_mode), &choice, 0);
	iniparser_set(setting->ini, "wireless:mode", wifi_mode[choice]);

	ret = cgiFormSelectSingle("safe", safe, sizeof(safe), &choice, 0);
	iniparser_set(setting->ini, "wireless:security", safe[choice]);

	ret = cgiFormString("password", string, sizeof(string));
	iniparser_set(setting->ini, "wireless:password", string);

	storeSettingInfo(setting);
}

/**
 * *  @brief       save softap setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doSoftapSave(struct setting_info *setting)
{
	char string[LEN64];
	char tmp[LEN64];
	cgiFormResultType ret;

	ret = cgiFormString("ssid", string, sizeof(string));
	tmp[0] = '\"';
	strcpy(tmp+1, string);
	tmp[strlen(string)+1] = '\"';
	tmp[strlen(string)+2] = '\0';
	iniparser_set(setting->ini, "softap:s_ssid", tmp);
	ret = cgiFormString("psd", string, sizeof(string));
	iniparser_set(setting->ini, "softap:s_password", string);

	storeSettingInfo(setting);
}

/**
 * *  @brief       save video setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doVideoSave(struct setting_info *setting)
{
	char string[LEN32];
	cgiFormResultType ret;
	int choice;
	bool bNeedRecoverQuality = true;
	const char * pStaticKbps = "static_kbps";

	//ret = cgiFormString("radioKbps", string, sizeof(string));
	//iniparser_set(setting->ini, "video:auto_kbps", string);

	//code stream 1
	ret = cgiFormSelectSingle("codedFormatOne", format, sizeof(format), &choice, 0);
	iniparser_set(setting->ini, "video:format1", format[choice]);

	ret = cgiFormSelectSingle("DPIOne", dpi, sizeof(dpi), &choice, 0);
	iniparser_set(setting->ini, "video:dpi1", dpi[choice]);

	ret = cgiFormString("kbpsOne", string, sizeof(string));
	if (strlen(string) == 0)
		strcpy(string, pStaticKbps);
	iniparser_set(setting->ini, "video:kbps_mode1", string);
	ret = cgiFormString("staticKpbs", string, sizeof(string));
	iniparser_set(setting->ini, "video:kbps1", string);
	ret = cgiFormString("iFrameRateOne", string, sizeof(string));
	iniparser_set(setting->ini, "video:group1", string);

	ret = cgiFormString("frameRateOne", string, sizeof(string));
	iniparser_set(setting->ini, "video:fps1", string);

	//code stream 2
	ret = cgiFormSelectSingle("codedFormatTwo", format, sizeof(format), &choice, 0);
	iniparser_set(setting->ini, "video:format2", format[choice]);
	if (choice == 0) bNeedRecoverQuality = false;

	ret = cgiFormSelectSingle("DPITwo", dpi2, sizeof(dpi2), &choice, 0);
	iniparser_set(setting->ini, "video:dpi2", dpi2[choice]);

	ret = cgiFormString("kbpsTwo", string, sizeof(string));
	if (strlen(string) == 0)
		strcpy(string, pStaticKbps);
	iniparser_set(setting->ini, "video:kbps_mode2", string);

	ret = cgiFormString("staticKpbs2", string, sizeof(string));
	iniparser_set(setting->ini, "video:kbps2", string);

	if (bNeedRecoverQuality) {
		ret = cgiFormSelectSingle("picQuality", quality, sizeof(quality), &choice, 0);
		iniparser_set(setting->ini, "video:quality", quality[choice]);
	}

	ret = cgiFormString("frameRateTwo", string, sizeof(string));
	iniparser_set(setting->ini, "video:fps2", string);

	ret = cgiFormString("iFrameRateTwo", string, sizeof(string));
	iniparser_set(setting->ini, "video:group2", string);

	storeSettingInfo(setting);
}

/**
 * *  @brief       save picture setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doPictureSave(struct setting_info *setting)
{
	char string[LEN32];
	cgiFormResultType ret;
	int choice;


	ret = cgiFormSelectSingle("picKbps", code_stream, sizeof(code_stream), &choice, 0);
	iniparser_set(setting->ini, "picture:video_index", code_stream[choice]);

	ret = cgiFormSelectSingle("ELHz", elhz, sizeof(elhz), &choice, 0);
	iniparser_set(setting->ini, "picture:el_hz", elhz[choice]);

	ret = cgiFormString("OSDName", string, sizeof(string));
	iniparser_set(setting->ini, "picture:osd_name", string);

	ret = cgiFormSelectSingle("OSDPlace",osd_place, sizeof(osd_place), &choice, 0);
	iniparser_set(setting->ini, "picture:osd_place", osd_place[choice]);

	ret = cgiFormSelectSingle("OSDTime",osd_time, sizeof(osd_time), &choice, 0);
	iniparser_set(setting->ini, "picture:osd_time", osd_time[choice]);

	storeSettingInfo(setting);
}

/**
 * *  @brief       save recoder setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doRecoderSave(struct setting_info *setting)
{
	char string[LEN32];
	cgiFormResultType ret;
	int choice;

	ret = cgiFormSelectSingle("recoderKbps", code_stream, sizeof(code_stream), &choice, 0);
	iniparser_set(setting->ini, "recoder:video_index", code_stream[choice]);

	ret = cgiFormSelectSingle("recoderTime", time, sizeof(time), &choice, 0);
	iniparser_set(setting->ini, "recoder:length", time[choice]);

	ret = cgiFormString("recoderOnTime", string, sizeof(string));
	iniparser_set(setting->ini, "recoder:time", string);

	storeSettingInfo(setting);
}

static void doFormatDiskSave(struct setting_info *setting)
{
	//nothing to save
}

/**
 * *  @brief       save user setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doUserSave(struct setting_info *setting)
{
	char string[LEN64];
	cgiFormResultType ret;

	ret = cgiFormString("userName", string, sizeof(string));
	iniparser_set(setting->ini, "user:name", string);

	ret = cgiFormString("password", string, sizeof(string));
	iniparser_set(setting->ini, "user:passwd", string);

	storeSettingInfo(setting);
}

static void doRecoverSave(struct setting_info *setting)
{
	//nothing to save
}

/**
 * *  @brief       save system setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doSystemSave(struct setting_info *setting)
{
	char string[LEN64];
	cgiFormResultType ret;

	ret = cgiFormString("password", string, sizeof(string));
	iniparser_set(setting->ini, "system:pw", string);

	storeSettingInfo(setting);
}

/**
 * *  @brief       save update app setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doUpdateAppSave(struct setting_info *setting)
{
}

/**
 * *  @brief       save update kernel setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doUpdateKerSave(struct setting_info *setting)
{
}

/**
 * *  @brief       save update reboot setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doUpdateRebootSave(struct setting_info *setting)
{
}


/**
 * *  @brief       save setting info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void doSettingSave(struct setting_info *setting)
{
	if (0 == strcmp(setting->mode, "ethernet_save")){
		doEthernetSave(setting);
	}
	else if (0 == strcmp(setting->mode, "wireless_save")){
		doWirelessSave(setting);
	}
	else if (0 == strcmp(setting->mode, "wireless_add_save")){
		doWirelessAddSave(setting);
	}
	else if (0 == strcmp(setting->mode, "softap_save")){
		doSoftapSave(setting);
	}
	else if (0 == strcmp(setting->mode, "video_save")){
		doVideoSave(setting);
	}
	else if (0 == strcmp(setting->mode, "picture_save")){
		doPictureSave(setting);
	}
	else if (0 == strcmp(setting->mode, "recoder_save")){
		doRecoderSave(setting);
	}
	else if (0 == strcmp(setting->mode, "format_disk")){
		doFormatDiskSave(setting);
	}
	else if (0 == strcmp(setting->mode, "user_save")){
		doUserSave(setting);
	}
	else if (0 == strcmp(setting->mode, "system_save")){
		doSystemSave(setting);
	}
	else if (0 == strcmp(setting->mode, "recover_save")){
		doRecoverSave(setting);
	}
	else if (0 == strcmp(setting->mode, "upapp_save")){
		doUpdateAppSave(setting);
	}
	else if (0 == strcmp(setting->mode, "upker_save")){
		doUpdateKerSave(setting);
	}
	else if (0 == strcmp(setting->mode, "reboot_save")){
		doUpdateRebootSave(setting);
	}
	else{
		// nothing to save
	}
}

/**
 * *  @brief       print html by mode
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void printHtmlByMode(struct setting_info *setting)
{
	if (0 == strcmp(setting->mode, "")){
		printEthernetHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "ethernet")){
		printEthernetHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "wireless")){
		printWirelessHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "softap")){
		printSoftapHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "video")){
		printVideoHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "picture")){
		printPictureHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "recoder")){
		printRecoderHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "user")){
		printUserHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "recover")){
		printRecoverHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "system")){
		printSystemHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "update")){
		printUpdateHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "reboot")){
		printRebootHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "update_app")){
		printUpdateAppHtml(setting, "");
	}
	else if (0 == strcmp(setting->mode, "update_ker")){
		printUpdateKerHtml(setting, "", 1);
	}
	else if (0 == strcmp(setting->mode, "ethernet_save")){
		printEthernetHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "wireless_save")){
		printWirelessHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "wireless_add_save")){
		printWirelessHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "softap_save")){
		printSoftapHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "video_save")){
		printVideoHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "picture_save")){
		printPictureHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "recoder_save")){
		printRecoderHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "format_disk")){
		printRecoderHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "user_save")){
		printUserHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "system_save")){
		printSystemHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "recover_save")){
		printRecoverHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "update_save")){
		printUpdateHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "upapp_save")){
		doAndPrintUpdateAppHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "upker_save")){
		doAndPrintUpdateKerHtml(setting);
	}
	else if (0 == strcmp(setting->mode, "reboot_save")){
		UpdateRebootInform(setting);
	}
	else if (0 == strcmp(setting->mode, "update_ker_burn")) {
		burnAndPrintfUpdateKerHtml(setting);
	}
	else{
		// nothing to display
	}
}

/**
 * *  @brief       do by mode
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void cgiDoSetting(struct setting_info *setting)
{
	if (0 == strcmp(setting->mode, "ethernet_save")){
		doEthernet(setting);
	}
	else if (0 == strcmp(setting->mode, "wireless_save")){
		doWireless(setting);
	}
	else if (0 == strcmp(setting->mode, "wireless_add_save")){
		doWirelessAdd(setting);
	}
	else if (0 == strcmp(setting->mode, "softap_save")){
		doSoftap(setting);
	}
	else if (0 == strcmp(setting->mode, "video_save")){
		doVideo(setting);
	}
	else if (0 == strcmp(setting->mode, "picture_save")){
		doPicture(setting);
	}
	else if (0 == strcmp(setting->mode, "recoder_save")){
		doRecoder(setting);
	}
	else if (0 == strcmp(setting->mode, "format_disk")){
		doFormatDisk(setting);
	}
	else if (0 == strcmp(setting->mode, "user_save")){
		doUser(setting);
	}
	else if (0 == strcmp(setting->mode, "recover_save")){
		doRecover(setting);
	}
	else if (0 == strcmp(setting->mode, "system_save")){
		doSystem(setting);
	}
	else if (0 == strcmp(setting->mode, "upapp_save")){
		doUpdateApp(setting);
	}
	else if (0 == strcmp(setting->mode, "upker_save")){
		doUpdateKer(setting);
	}
	else if (0 == strcmp(setting->mode, "reboot_save")){
		doReboot(setting);
	}
	else{
		// nothing todo
	}
}

/**
 * *  @brief       init the system V semaphore
 * *  @author
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return	   none
 * */
static int init_systemv_sem()
{
	union semun seminfo;
	unsigned short array[SEM_NUMS] = {1, 0, 1, 1, 0, 1};

	sem_key = ftok(CAMERA, SEM_PROJ_ID); //use camera.ini to generate the key
	if (sem_key < 0) {
		print_error("%s->%s:ftok fail!", __FILE__, __func__);
		return -1;
	}

	if ((sem_id = semget(sem_key, 0, 0)) < 0) {
		if ((sem_id = semget(sem_key, SEM_NUMS, IPC_CREAT | 0666)) < 0) {
			print_error("%s->%s:semget fail, error %d!", __FILE__, __func__, errno);
			return -1;
		}

		seminfo.array = array;
		if (semctl(sem_id, 0, SETALL, seminfo) < 0) {
			print_error("%s->%s:semctl fail, error %d!", __FILE__, __func__, errno);
			return -1;
		}
	}

	return 0;
}

/**
 * *  @brief       do cgi setting
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   setting
 * *  @return      none
 * */
static void cgiSetting(struct setting_info *setting)
{
	doSettingSave(setting);

	if (init_systemv_sem() < 0) return;

	switch(fork())
	{
	case -1:
		fclose(cgiOut);
		print_error("%s->%s:fork fail!", __FILE__, __func__);
		print_error_2html("%s->%s:fork fail!", __FILE__, __func__);
		break;
	case 0:
		settingDebug("I am child, my id is %d\n", getpid());
		printHtmlByMode(setting);
		fflush(cgiOut);
		break;
	default:
		fclose(cgiOut);
		settingDebug("I am parent, my id is %d\n", getpid());
		wait(NULL);
		sleep(1);
		cgiDoSetting(setting);
		break;
	}
}

/**
 * *  @brief       cgi main
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   void
 * *  @return      ret
 * */
int cgiMain()
{
	int ret = 0;
	struct setting_info *setting;

	cgiRestoreStdin();
	cgiRestoreStdout();
	cgiRestoreStderr();

	setting = (struct setting_info *)malloc(sizeof(struct setting_info));
	if (NULL == setting){
		print_error("ERROR: There is no more memory!\n");
		print_error_2html("ERROR: There is no more memory!\n");
		ret = -ENOMEM;
		goto nomem_out;
	}
	memset(setting, 0, sizeof(struct setting_info));

	setting->ini = iniparser_load(CAMERA);
	if (NULL == setting->ini){
		print_error("ERROR: load config file fail!\n");
		print_error_2html("ERROR: load config file fail!\n");
		ret = -ENODATA;
		goto nodata_out;
	}
	loadSettingInfo(setting);

	if (cgiLogin(setting)){
		goto finish_out;
	}

	cgiFormString("mode", setting->mode, sizeof(setting->mode));
	settingDebug("\n%s: mode is %s!\n", __func__, setting->mode);
	cgiSetting(setting);

finish_out:
	settingDebug("%s: Process %d finish!\n", __func__, getpid());
	iniparser_freedict(setting->ini);
	free(setting);
	return 0;
nodata_out:
	free(setting);
nomem_out:
	return ret;
}
