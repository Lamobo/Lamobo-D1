/*
 *  login.c, a wifi login html page and cgi
 *  Copyright (C) 2012 anyka <gao_wangsheng@anyka.oa>
 *  Created by gao_wangsheng  2012-8-8
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
#include <sys/types.h>
#include <sys/stat.h>

#include "cgic.h"
#include "cgi_anyka.h"


static void cgiDispLoginWeb(void)
{
	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
	fprintf(cgiOut, "<html>\n");
	fprintf(cgiOut, "<head><meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">\n");
	fprintf(cgiOut, "<TITLE>%s</TITLE>\n", TITLE);
	fprintf(cgiOut, "</head>\n");
	fprintf(cgiOut, "<body topmargin=\"0\" leftmargin=\"0\" color=\"ffffff\" style=\"background-color: black; font-family: Arial;\" marginheight=\"0\" marginwidth=\"0\">\n");
	fprintf(cgiOut, "<form method=\"POST\" action=\"%s\" >\n", LOGIN);
	fprintf(cgiOut, "<table style=\"width: 960px; height: 700px; text-align: left; margin-left: auto; margin-right: auto;\" id=\"___01\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" align=\"center\">\n");
	fprintf(cgiOut, "<tbody>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td style=\"height: 25px; width: 960px;\" colspan=\"3\" background=\"../images/login_01.gif\"> <br></td></tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td style=\"height: 37px; width: 960px;\" colspan=\"3\" background=\"../images/login_02.gif\">\n");
	fprintf(cgiOut, "<div style=\"font-size: 20pt;\">&nbsp;&nbsp;&nbsp;Welcome to <font color=\"#69b40f\">%s</font></div>\n",
		WELCOME);
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td style=\"height: 217px; width: 960px;\" colspan=\"3\" background=\"../images/login_03.gif\"> <br>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td style=\"height: 175px; width: 267px;\" rowspan=\"3\" background=\"../images/login_04.gif\"> <br>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td background=\"../images/login_05.gif\" valign=\"center\">\n");
	fprintf(cgiOut, "<div style=\"font-size: 14pt; color: rgb(255, 255, 255); \">Welcome to %s</div>\n",
		WELCOME);
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "<td style=\"height: 175px; width: 268px;\" rowspan=\"3\" background=\"../images/login_06.gif\"> <br>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td style=\"height: 104px; width: 425px; text-align: center;\" background=\"../images/login_07.gif\">\n");
	fprintf(cgiOut, "<div style=\"font-size: 12pt; color: rgb(35, 35, 35);\">Please enter your password&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<br>\n");
	fprintf(cgiOut, "<input maxlength=\"64\" name=\"password\" style=\"padding:3px 0px 0px 6px;border: 0px none ; background: transparent url(../images/input/common_panel_input_base.png) repeat scroll 0%% 50%%; -moz-background-clip: initial; -moz-background-origin: initial; -moz-background-inline-policy: initial; width: 217px; font-family: Arial; height: 23px;\" type=\"password\"></div>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td style=\"height: 47px; width: 425px; text-align: right;\" background=\"../images/login_08.gif\">\n");
	fprintf(cgiOut, "<input value=\"OK\" style=\"border: 0px none ; background: transparent url(../images/button/common_gn_button_medium_rest.png) repeat scroll 0%% 50%%; -moz-background-clip: initial; -moz-background-origin: initial; -moz-background-inline-policy: initial; width: 149px; font-family: Arial; height: 28px; font-weight: bold; color: rgb(255, 255, 255);\" type=\"submit\">\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td style=\"height: 246px; width: 960px;\" colspan=\"3\" background=\"../images/login_09.jpg\"> <br>\n");
	fprintf(cgiOut, "</td>\n");
	fprintf(cgiOut, "</tr>\n");
	fprintf(cgiOut, "</tbody>\n");
	fprintf(cgiOut, "</table>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
}

static void cgiDispLoginMobile(void)
{
	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//WAPFORUM//DTD XHTML Mobile 1.0//EN\" \"http://www.openmobilealliance.org/DTD/xhtml-mobile10.dtd\" >\n");
	fprintf(cgiOut, "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	fprintf(cgiOut, "<head>\n");
	fprintf(cgiOut, "<meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">\n");
	fprintf(cgiOut, "<link rel=\"stylesheet\"  type=\"text/css\"  href=\"../style_dev.css\"  media=\"screen\">\n");
	fprintf(cgiOut, "<TITLE>%s</TITLE>\n", TITLE);
	fprintf(cgiOut, "</head>\n");
	fprintf(cgiOut, "<body style=\"max-width:386px;font-family:Droid, Arial;\" marginheight=\"0\" marginwidth=\"0\">\n");
	fprintf(cgiOut, "<form method=\"POST\" action=\"%s\" >\n", LOGIN);
	fprintf(cgiOut, "<table BORDER=\"0\" CELLPADDING=\"0\" CELLSPACING=\"0\">\n");
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td class=\"title\"></td></tr>\n");
	fprintf(cgiOut, "<tr><td class=\"header_top\"></td></tr>\n");
	fprintf(cgiOut, "<tr><td class=\"txt26\" style=\"padding-left:14px;\">Welcome to <font color=\"#69b40f\">%s</font></td></tr>\n",
		WELCOME);
	fprintf(cgiOut, "<tr><td class=\"txt20\" style=\"padding-left:14px;\">Please enter your password</td></tr>\n");
	fprintf(cgiOut, "<tr><td style=\"max-width:386px; width: 386px; height: 55px; vertical-align: middle;\">&nbsp;&nbsp;\n");
	fprintf(cgiOut, "<input maxlength=\"64\" name=\"password\" style=\"padding:10px 0px 0px 10px; font-size: 18pt;border: 0px none ; background: transparent url(../images/common_input_rest.png) repeat scroll 0%% 50%%;  width: 356px; height: 44px;\" type=\"password\"> </td></tr>\n");
	fprintf(cgiOut, "<tr><td style=\"width: 386px; height:300px\"></td></tr>\n");
	fprintf(cgiOut, "<tr><td class=\"header_bottom\" ></td></tr>\n");
	fprintf(cgiOut, "<tr><td align=\"center\" class=\"botton\">\n");
	fprintf(cgiOut, "<input value=\"OK\" class=\"btntext\"	type=\"submit\"></td></tr>\n");
	fprintf(cgiOut, "</form>\n");
	fprintf(cgiOut, "</body>\n");
	fprintf(cgiOut, "</html>\n");
	fflush(cgiOut);
}

static void cgiDispLogin(void)
{
	int IsMobile;

	IsMobile = cgiIsMobile();
	if (IsMobile)
	{
		cgiDispLoginMobile();
	}
	else
	{
		cgiDispLoginWeb();
	}
}

static void cgiCookieSet(char *name, char *value)
{
	if (strlen(name))
	{
		fprintf(cgiOut, "Set-Cookie: %s=%s\r\n",	name, value);			
	}
}

static int cgiGetPassword(char *pw)
{
	FILE *fp = NULL;
	char buf[AP_SIZE];
	
	memset(buf, '\0', sizeof(buf));
	fp = fopen(PASSWD, "r");
	if (NULL == fp)
	{
		print_error("%s->%s:open file %s fail!\n", __FILE__, __func__, PASSWD);
		print_error_2html("%s->%s:open file %s fail. Please retry!", 
				__FILE__, __func__, PASSWD);
		return -1;
	}
	
	fread(buf, sizeof(char), AP_SIZE, fp);
	fclose(fp);
	cgiGetVal(buf, "passphrase=", pw);
	return 0;
}

int cgiMain() 
{
	char password[LEN];
	char pw[LEN];
	cgiFormResultType result;

	cgiRestoreStdin();
	cgiRestoreStdout();
//	cgiRestoreStderr();

	if (cgiCheckLogin() == true)
	{
		cgiRedirect(SETTING);
		return 0;
	}

	cgiDebug("\n%s:%s:\n", __FILE__, __func__);
	result = cgiFormString("password", password, sizeof(password));
	switch(result)
	{
	case cgiFormSuccess:
		if (cgiGetPassword(pw))
		{
			return -1;
		}
		
		if (!strcmp(password, pw))
		{
			cgiCookieSet(CNAME, LOK);
			cgiRedirect(SETTING);
		}
		else
		{
			cgiCookieSet(CNAME, LFAIL);
			cgiDispLogin();
		}
		break;

	default:
		cgiDispLogin();
		break;
	}

	return 0;
}


