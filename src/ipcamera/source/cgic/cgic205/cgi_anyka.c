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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "cgic.h"
#include "cgi_anyka.h"

/**
 * *  @brief       redirect to page
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   page
 * *  @return      none
 * */
void cgiRedirect(char *page)
{
	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<HTML>\n");
	fprintf(cgiOut, "<HEAD>\n");
	fprintf(cgiOut, "<meta http-equiv=\"refresh\" content=\"0;url=%s\">",  page);
	fprintf(cgiOut, "</HEAD>\n");
	fprintf(cgiOut, "</HTML>\n");
	fflush(cgiOut);
}

/**
 * *  @brief       check client type
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   void
 * *  @return
 * */
int cgiIsMobile(void)
{
	return strstr(cgiUserAgent, "mobile") 
		|| strstr(cgiUserAgent, "Mobile")
		|| strstr(cgiUserAgent, "android")
		|| strstr(cgiUserAgent, "Android")
		|| strstr(cgiUserAgent, "iphone")
		|| strstr(cgiUserAgent, "Iphone");
}

/**
 * *  @brief       get val from str
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   str, name, val 
 * *  @return	   1 on success	
 * */
int cgiGetVal(const char *str, const char *name, char *val)
{
	str = strstr(str, name);
	if(str)
	{
		for(str += strlen(name); (*str) && (*str != '"') && (*str != '\n'); )
		{
			*val++ = *str++;
		}
	}
	
	*val = '\0';
	return str != NULL;
}

/**
 * *  @brief       check security
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   str, name, val 
 * *  @return	   none zero on success	
 * */
int cgiCheckSecurity(char *buf)
{
	int val = 0;

	if (strstr(buf, "WPA"))
	{
		val = WPA;
	}
	else if (strstr(buf, "WEP") || strstr(buf, "Encryption key:on"))
	{
		val = WEP;
	}
	else
	{
		val = OPEN;
	}

	return val;
}

/**
 * *  @brief       scan ap info to file
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   void
 * *  @return	   void	
 * */
void cgiScanOtherAp2File(void)
{
	char cmd[LEN];

	// scan other AP to ap_list
	sprintf(cmd, "/sbin/iwlist wlan0 scanning > %s", AP);
	system(cmd);
}

/**
 * *  @brief       get softap info to file
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   void
 * *  @return	   void	
 * */
void cgiScanMyAp2File(void)
{
	char cmd[LEN];

	// scan My AP to ap_list
	sprintf(cmd, "/usr/bin/hostapd_cli -iwlan0 get_config > %s", MY_AP);
	system(cmd);
}

/**
 * *  @brief       init ap info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   ap
 * *  @return	   void	
 * */
static void cgiInitMyAp(struct ap *ap)
{
	ap->index = 0;
	ap->security = 0;
	memset(ap->address, '\0', sizeof(ap->address));
	memset(ap->ssid, '\0', sizeof(ap->ssid));
	memset(ap->password, '\0', sizeof(ap->password));
	memset(ap->protocol, '\0', sizeof(ap->protocol));
	memset(ap->mode, '\0', sizeof(ap->mode));
	memset(ap->frequency, '\0', sizeof(ap->frequency));
	memset(ap->en_key, '\0', sizeof(ap->en_key));
	memset(ap->bit_rates, '\0', sizeof(ap->bit_rates));
	memset(ap->sig_level, '\0', sizeof(ap->sig_level));
	memset(ap->address, '\0', sizeof(ap->address));
}

/**
 * *  @brief       get soft ap info
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   ap
 * *  @return	   none zero on success
 * */
int cgiGetMyAp(struct ap *ap)
{
	FILE *fp = NULL;
	char buf[AP_SIZE];
	
	cgiInitMyAp(ap);
	memset(buf, '\0', sizeof(buf));
	
	fp = fopen(MY_AP, "r");
	if (NULL != fp)
	{
		fread(buf, sizeof(char), AP_SIZE, fp);
		fclose(fp);
		cgiGetVal(buf, "bssid=", ap->address);
		if (!strlen(ap->address))
		{
			strncpy(ap->address, "00:11:22:33:44:55", sizeof(ap->address));
		}
	}
	else
	{
		print_error("%s->%s:open file %s fail!\n", __FILE__,__func__, MY_AP);
	}

	memset(buf, '\0', sizeof(buf));
	fp = fopen(APCONF, "r");
	if (NULL != fp)
	{
		fread(buf, sizeof(char), AP_SIZE, fp);
		fclose(fp);
		cgiGetVal(buf, "ssid=", ap->ssid);
		cgiGetVal(buf, "passphrase=", ap->password);
		ap->security = cgiCheckSecurity(buf);		
	}
	else
	{
		print_error("%s->%s:open file %s fail!\n", __FILE__, __func__, APCONF);
	}	

	settingDebug("Ad=%s; ssid=%s; sec=%d; pw=%s\n",
		ap->address, ap->ssid, ap->security, ap->password);

	return strlen(ap->ssid);
}

/**
 * *  @brief       check login
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   void
 * *  @return	   none zero on success
 * */
int cgiCheckLogin(void)
{
	char cvalue[LINE];

	cgiCookieString(CNAME, cvalue, sizeof(cvalue));
	if (strcmp(cvalue, LOK) != 0)
	{
		return false;
	}

	return true;
}

/**
 * *  @brief       restore cgi stdin
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   void
 * *  @return	   void
 * */
void cgiRestoreStdin(void)
{
	int cgiInFd;
	int cigInFdCopy;
	
	cgiInFd = fileno(cgiIn);
	cigInFdCopy = dup(cgiInFd);
	fclose(cgiIn);
	freopen(CONSOLE, "r", stdin);
	cgiIn = fdopen(cigInFdCopy, "r");
}

/**
 * *  @brief       restore cgi stdout
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   void
 * *  @return	   void
 * */
void cgiRestoreStdout(void)
{
	int cgiOutFd;
	int cigOutFdCopy;
	
	cgiOutFd = fileno(cgiOut);
	cigOutFdCopy = dup(cgiOutFd);
	fclose(cgiOut);
	freopen(CONSOLE, "w", stdout);
	cgiOut = fdopen(cigOutFdCopy, "w");
}

/**
 * *  @brief       restore cgi stderr
 * *  @author      gao wangsheng
 * *  @date        2013-5-9
 * *  @param[in]   void
 * *  @return	   void
 * */
void cgiRestoreStderr(void)
{
	freopen(CONSOLE, "w", stderr);
}


