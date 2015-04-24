#include "inisetting.h"
#define INI_CAMERA "/etc/jffs2/camera.ini"
#define INI_AKI		"/etc/jffs2/akiserver.ini"


static struct setting_info setting;
static struct akisetting_info akiset;
static int nInit = 0;
static int nInitaki = 0;

int IniSetting_init()
{
	setting.ini = iniparser_load(INI_CAMERA);
	if (NULL == setting.ini){
		print_error("ERROR: load config file fail!\n");
		return -1;
	}
	
	settingDebug("load camera ini file:%s success\n", INI_CAMERA);
	//video info
	#if 0
	setting.video.format1 = iniparser_getstring(setting.ini, "video:format1", NULL);
	setting.video.dpi1 = iniparser_getstring(setting.ini, "video:dpi1", NULL);
	setting.video.kbps1 = iniparser_getstring(setting.ini, "video:kbps1", NULL);
	setting.video.kbps_mode1 = iniparser_getstring(setting.ini, "video:kbps_mode1", NULL);
	setting.video.minqp1 = iniparser_getstring(setting.ini, "video:minqp1", NULL);
	setting.video.maxqp1 = iniparser_getstring(setting.ini, "video:maxqp1", NULL);
	setting.video.fps1 = iniparser_getstring(setting.ini, "video:fps1", NULL);
	setting.video.format2= iniparser_getstring(setting.ini, "video:format2", NULL);
	setting.video.dpi2 = iniparser_getstring(setting.ini, "video:dpi2", NULL);
	setting.video.kbps2 = iniparser_getstring(setting.ini, "video:kbps2", NULL);
	setting.video.kbps_mode2 = iniparser_getstring(setting.ini, "video:kbps_mode2", NULL);
	setting.video.minqp2 = iniparser_getstring(setting.ini, "video:minqp2", NULL);
	setting.video.maxqp2 = iniparser_getstring(setting.ini, "video:maxqp2", NULL);
	setting.video.fps2 = iniparser_getstring(setting.ini, "video:fps2", NULL);
	setting.video.video_kbps = iniparser_getstring(setting.ini, "video:video_kbps", NULL);

	setting.recoder.length = iniparser_getstring(setting.ini, "recoder:length", NULL);
	setting.recoder.time = iniparser_getstring(setting.ini, "recoder:time", NULL);
	#endif
	nInit = 1;
	return nInit;
}
void IniSetting_destroy()
{
	if(nInit == 1)
		iniparser_freedict(setting.ini);
		
	nInit = 0;
	return;
}
int IniSetting_save()
{
	FILE *fp = NULL;
	
	if(!nInit)
		return -2;
	fp = fopen(INI_CAMERA, "w");
	if (NULL == fp){
		print_error("%s:open %s file fail!", __func__, INI_CAMERA);
		return -1;
	}

	iniparser_dump_ini(setting.ini, fp);
	fclose(fp);
	settingDebug("save camera ini file:%s success\n", INI_CAMERA);
	return 1;
}

struct video_info * IniSetting_GetVideoInfo()
{
	if(!nInit)
		return NULL;
	
	setting.video.format1 = iniparser_getstring(setting.ini, "video:format1", NULL);
	setting.video.dpi1 = iniparser_getstring(setting.ini, "video:dpi1", NULL);
	setting.video.kbps1 = iniparser_getstring(setting.ini, "video:kbps1", NULL);
	setting.video.kbps_mode1 = iniparser_getstring(setting.ini, "video:kbps_mode1", NULL);
	setting.video.group1 = iniparser_getstring(setting.ini, "video:group1", NULL);
	setting.video.fps1 = iniparser_getstring(setting.ini, "video:fps1", NULL);
	setting.video.format2= iniparser_getstring(setting.ini, "video:format2", NULL);
	setting.video.dpi2 = iniparser_getstring(setting.ini, "video:dpi2", NULL);
	setting.video.kbps2 = iniparser_getstring(setting.ini, "video:kbps2", NULL);
	setting.video.kbps_mode2 = iniparser_getstring(setting.ini, "video:kbps_mode2", NULL);
	setting.video.quality = iniparser_getstring(setting.ini, "video:quality", NULL);
	setting.video.group2 = iniparser_getstring(setting.ini, "video:group2", NULL);
	setting.video.fps2 = iniparser_getstring(setting.ini, "video:fps2", NULL);
	setting.video.video_kbps = iniparser_getstring(setting.ini, "video:video_kbps", NULL);
	
	settingDebug("setting.video.format1:%s\n", setting.video.format1);
	settingDebug("setting.video.dpi1:%s\n", setting.video.dpi1);
	settingDebug("setting.video.kbps1:%s\n", setting.video.kbps1);
	settingDebug("setting.video.kbps_mode1:%s\n", setting.video.kbps_mode1);
	settingDebug("setting.video.group1:%s\n", setting.video.group1);
	
	settingDebug("setting.video.fps1:%s\n", setting.video.fps1);

	settingDebug("setting.video.format2:%s\n", setting.video.format2);
	settingDebug("setting.video.dpi2:%s\n", setting.video.dpi2);
	settingDebug("setting.video.kbps2:%s\n", setting.video.kbps2);
	settingDebug("setting.video.kbps_mode2:%s\n", setting.video.kbps_mode2);
	settingDebug("setting.video.minqp2:%s\n", setting.video.quality);
	settingDebug("setting.video.maxqp2:%s\n", setting.video.group2);
	settingDebug("setting.video.fps2:%s\n", setting.video.fps2);
	
	settingDebug("setting.video.video_kbps:%s\n", setting.video.video_kbps);
	
	return &(setting.video);
}

struct recoder_info * IniSetting_GetRecordInfo()
{
	if(!nInit)
		return NULL;
	setting.recoder.video_index = iniparser_getstring(setting.ini, "recoder:video_index", NULL);
	setting.recoder.length = iniparser_getstring(setting.ini, "recoder:length", NULL);
	setting.recoder.time = iniparser_getstring(setting.ini, "recoder:time", NULL);

	
	settingDebug("ssetting.recoder.length:%s\n", setting.recoder.length);
	settingDebug("setting.recoder.time:%s\n", setting.recoder.time);
	
	return &(setting.recoder);
}

struct picture_info *IniSetting_GetPictureInfo()
{
	if(!nInit)
		return NULL;

	setting.picture.video_index = iniparser_getstring(setting.ini, "picture:video_index", NULL);
	setting.picture.el_hz = iniparser_getstring(setting.ini, "picture:el_hz", NULL);
	setting.picture.osd_name = iniparser_getstring(setting.ini, "picture:osd_name", NULL);
	setting.picture.osd_place = iniparser_getstring(setting.ini, "picture:osd_place", NULL);
	setting.picture.osd_time = iniparser_getstring(setting.ini, "picture:osd_time", NULL);

	return &(setting.picture);
}

#if 0
struct system_info* IniSetting_GetSysInfo()
{
	if(!nInit)
		return NULL;
	
	setting.system.sn = iniparser_getstring(setting.ini, "system:sn", NULL);
	setting.system.pw = iniparser_getstring(setting.ini, "system:pw", NULL);
	setting.system.product = iniparser_getstring(setting.ini, "system:product", NULL);
	setting.system.type = iniparser_getstring(setting.ini, "system:type", NULL);
	setting.system.serverip = iniparser_getstring(setting.ini, "system:serverip", NULL);
	setting.system.freq = iniparser_getstring(setting.ini, "system:freq", NULL);
	setting.system.serveradd = iniparser_getstring(setting.ini, "system:serveradd", NULL);
	setting.system.port = iniparser_getstring(setting.ini, "system:port", NULL);
	
	settingDebug("setting.system.sn:%s\n", setting.system.sn);
	settingDebug("setting.system.pw:%s\n", setting.system.pw);
	settingDebug("setting.system.product:%s\n", setting.system.product);
	settingDebug("setting.system.type:%s\n", setting.system.type);
	settingDebug("setting.system.serverip:%s\n", setting.system.serverip);
	settingDebug("setting.system.serveradd:%s\n", setting.system.serveradd);
	settingDebug("setting.system.port:%s\n", setting.system.port);
	
	return &(setting.system);
}
struct user_info* IniSetting_GetUsrInfo()
{
	if(!nInit)
		return NULL;
		
	setting.user.name = iniparser_getstring(setting.ini, "user:name", NULL);
	setting.user.passwd = iniparser_getstring(setting.ini, "user:passwd", NULL);
	setting.user.confirmpw = iniparser_getstring(setting.ini, "user:confirmpw", NULL);
	setting.user.timezone = iniparser_getstring(setting.ini, "user:timezone", NULL);
	
	settingDebug("setting.user.name:%s\n", setting.user.name);
	settingDebug("setting.user.passwd:%s\n", setting.user.passwd);
	settingDebug("setting.user.confirmpw:%s\n", setting.user.confirmpw);
	settingDebug("setting.user.timezone:%s\n", setting.user.timezone);
	
	return &(setting.user);
}
struct recoder_info* IniSetting_GetRecInfo()
{
	if(!nInit)
		return NULL;
		
	setting.recoder.repeat = iniparser_getstring(setting.ini, "recoder:repeat", NULL);
	setting.recoder.policy = iniparser_getstring(setting.ini, "recoder:policy", NULL);
	setting.recoder.week = iniparser_getstring(setting.ini, "recoder:week", NULL);
	setting.recoder.time1_start = iniparser_getstring(setting.ini, "recoder:time1_start", NULL);
	setting.recoder.time1_end = iniparser_getstring(setting.ini, "recoder:time1_end", NULL);
	setting.recoder.time2_start = iniparser_getstring(setting.ini, "recoder:time2_start", NULL);
	setting.recoder.time2_end = iniparser_getstring(setting.ini, "recoder:time2_end", NULL);
	setting.recoder.time3_start = iniparser_getstring(setting.ini, "recoder:time3_start", NULL);
	setting.recoder.time3_end = iniparser_getstring(setting.ini, "recoder:time3_end", NULL);
	setting.recoder.time4_start = iniparser_getstring(setting.ini, "recoder:time4_start", NULL);
	setting.recoder.time4_end = iniparser_getstring(setting.ini, "recoder:time4_end", NULL);
	
	settingDebug("setting.recoder.repeat:%s\n", setting.recoder.repeat);
	settingDebug("setting.recoder.policy:%s\n", setting.recoder.policy);
	settingDebug("setting.recoder.week:%s\n", setting.recoder.week);
	settingDebug("setting.recoder.time1_start:%s\n", setting.recoder.time1_start);
	settingDebug("setting.recoder.time1_end:%s\n", setting.recoder.time1_end);
	settingDebug("setting.recoder.time2_start:%s\n", setting.recoder.time2_start);
	settingDebug("setting.recoder.time2_end:%s\n", setting.recoder.time2_end);
	settingDebug("setting.recoder.time3_start:%s\n", setting.recoder.time3_start);
	settingDebug("setting.recoder.time4_start:%s\n", setting.recoder.time4_start);
	settingDebug("setting.recoder.time4_end:%s\n", setting.recoder.time4_end);
	settingDebug("setting.recoder.free_space:%s\n", setting.recoder.free_space);
	
	return &(setting.recoder);
}
struct wireless_info* IniSetting_GetWilInfo()
{
	if(!nInit)
		return NULL;
	
	setting.wireless.open = iniparser_getstring(setting.ini, "wireless:open", NULL);
	setting.wireless.ssid = iniparser_getstring(setting.ini, "wireless:ssid", NULL);
	setting.wireless.mode = iniparser_getstring(setting.ini, "wireless:mode", NULL);
	setting.wireless.security = iniparser_getstring(setting.ini, "wireless:security", NULL);
	setting.wireless.password = iniparser_getstring(setting.ini, "wireless:password", NULL);
	
	settingDebug("setting.wireless.open:%s\n", setting.wireless.open);
	settingDebug("setting.wireless.ssid:%s\n", setting.wireless.ssid);
	settingDebug("setting.wireless.mode:%s\n", setting.wireless.mode);
	settingDebug("setting.wireless.security:%s\n", setting.wireless.security);
	settingDebug("setting.wireless.password:%s\n", setting.wireless.password);
	//ap_shop
	//settingDebug("setting.wireless.open:%s\n", setting.wireless.open);
	return &(setting.wireless);
}
struct ethernet_info* IniSetting_GetEthInfo()
{
	if(!nInit)
		return NULL;
	
	setting.ethernet.dhcp = iniparser_getstring(setting.ini, "ethernet:dhcp", NULL);
	//setting->ethernet.upnp = iniparser_getstring(setting.ini, "ethernet:upnp", NULL);
	setting.ethernet.ipaddr = iniparser_getstring(setting.ini, "ethernet:ipaddr", NULL);
	//setting->ethernet.httport = iniparser_getstring(setting.ini, "ethernet:httport", NULL);
	setting.ethernet.netmask = iniparser_getstring(setting.ini, "ethernet:netmask", NULL);
	setting.ethernet.gateway = iniparser_getstring(setting.ini, "ethernet:gateway", NULL);
	setting.ethernet.firstdns = iniparser_getstring(setting.ini, "ethernet:firstdns", NULL);
	setting.ethernet.backdns = iniparser_getstring(setting.ini, "ethernet:backdns", NULL);
	//setting->ethernet.macaddr = iniparser_getstring(setting.ini, "ethernet:macaddr", NULL);
	
	settingDebug("setting.ethernet.dhcp:%s\n", setting.ethernet.dhcp);
	settingDebug("setting.ethernet.ipaddr:%s\n", setting.ethernet.ipaddr);
	settingDebug("setting.ethernet.netmask:%s\n", setting.ethernet.netmask);
	settingDebug("setting.ethernet.gateway:%s\n", setting.ethernet.gateway);
	settingDebug("setting.ethernet.firstdns:%s\n", setting.ethernet.firstdns);
	settingDebug("setting.ethernet.backdns:%s\n", setting.ethernet.backdns);
	
	return &(setting.ethernet);
}
struct global_info* IniSetting_GetGloInfo()
{
	if(!nInit)
		return NULL;
	
	setting.global.title = iniparser_getstring(setting.ini, "global:title", NULL);
	setting.global.welcome = iniparser_getstring(setting.ini, "global:welcome", NULL);
	
	settingDebug("setting.global.title:%s\n", setting.global.title);
	settingDebug("setting.global.welcome:%s\n", setting.global.welcome);
	
	return &(setting.global);
}

int IniSetting_SetSysInfo(struct system_info* sysinfo)
{
	if(!sysinfo)
		return -2;
	if(!nInit)
		return -1;
	
	iniparser_set(setting.ini, "system:sn", sysinfo->sn);
	iniparser_set(setting.ini, "system:pw", sysinfo->pw);
	iniparser_set(setting.ini, "system:product", sysinfo->product);
	iniparser_set(setting.ini, "system:type", sysinfo->type);
	iniparser_set(setting.ini, "system:serverip", sysinfo->serverip);
	iniparser_set(setting.ini, "system:freq", sysinfo->freq);
	iniparser_set(setting.ini, "system:serveradd", sysinfo->serveradd);
	iniparser_set(setting.ini, "system:prot", sysinfo->port);
	
	return 1;
}
int IniSetting_SetUsrInfo(struct user_info* userinfo)
{
	if(!userinfo)
		return -2;
	if(!nInit)
		return -1;
		
	iniparser_set(setting.ini, "user:name", userinfo->name);
	iniparser_set(setting.ini, "user:passwd", userinfo->passwd);
	iniparser_set(setting.ini, "user:confirmpw", userinfo->confirmpw);
	iniparser_set(setting.ini, "user:timezone", userinfo->timezone);

	return 1;		
}
int IniSetting_SetRecInfo(struct recoder_info* recinfo)
{
	if(!recinfo)
		return -2;
	if(!nInit)
		return -1;
		
	iniparser_set(setting.ini, "recoder:repeat", recinfo->repeat);
	iniparser_set(setting.ini, "recoder:policy", recinfo->policy);
	iniparser_set(setting.ini, "recoder:week", recinfo->week);
	iniparser_set(setting.ini, "recoder:time1_start", recinfo->time1_start);
	iniparser_set(setting.ini, "recoder:time1_end", recinfo->time1_end);
	iniparser_set(setting.ini, "recoder:time2_start", recinfo->time2_start);
	iniparser_set(setting.ini, "recoder:time2_end", recinfo->time2_end);
	iniparser_set(setting.ini, "recoder:time3_start", recinfo->time3_start);
	iniparser_set(setting.ini, "recoder:time3_end", recinfo->time3_end);
	iniparser_set(setting.ini, "recoder:time4_start", recinfo->time4_start);
	iniparser_set(setting.ini, "recoder:time4_end", recinfo->time4_end);
	
	return 1;
}
int IniSetting_SetWilInfo(struct wireless_info* wilinfo)
{
	if(!wilinfo)
		return -2;
	if(!nInit)
		return -1;
	
	iniparser_set(setting.ini, "wireless:open", wilinfo->open);
	iniparser_set(setting.ini, "wireless:ssid", wilinfo->ssid);
	iniparser_set(setting.ini, "wireless:mode", wilinfo->mode);
	iniparser_set(setting.ini, "wireless:security", wilinfo->security);
	iniparser_set(setting.ini, "wireless:password", wilinfo->password);
	
	return 1;	
}
int IniSetting_SetEthInfo(struct ethernet_info* ethinfo)
{
	if(!ethinfo)
		return -2;
	if(!nInit)
		return -1;
		
	iniparser_set(setting.ini, "ethernet:dhcp", ethinfo->dhcp);
	iniparser_set(setting.ini, "ethernet:ipaddr", ethinfo->ipaddr);
	iniparser_set(setting.ini, "ethernet:netmask", ethinfo->netmask);
	iniparser_set(setting.ini, "ethernet:gateway", ethinfo->gateway);
	iniparser_set(setting.ini, "ethernet:firstdns", ethinfo->firstdns);
	iniparser_set(setting.ini, "ethernet:backdns", ethinfo->backdns);
	
	return 1;
}
int IniSetting_SetGloInfo(struct global_info* gloinfo)
{
	if(!gloinfo)
		return -2;
	if(!nInit)
		return -1;
	
	iniparser_set(setting.ini, "global:title", gloinfo->title);
	iniparser_set(setting.ini, "global:welcome", gloinfo->welcome);
	
	return 1;
}

#endif


int IniSetting_aki()
{
	akiset.ini = iniparser_load(INI_AKI);
	if (NULL == akiset.ini){
		print_error("ERROR: load config file fail!\n");
		return -1;
	}
	
	settingDebug("load akiserver ini file:%s success\n", INI_AKI);
	//video info
	akiset.occ.start_xpos1 = iniparser_getstring( akiset.ini, "occ:start_xpos1", NULL);
	akiset.occ.start_ypos1 = iniparser_getstring( akiset.ini, "occ:start_ypos1", NULL);
	akiset.occ.end_xpos1   = iniparser_getstring( akiset.ini, "occ:end_xpos1", NULL);
	akiset.occ.end_ypos1   = iniparser_getstring( akiset.ini, "occ:end_ypos1", NULL);
	akiset.occ.enable1     = iniparser_getstring( akiset.ini, "occ:enable1", NULL);
	akiset.occ.start_xpos2 = iniparser_getstring( akiset.ini, "occ:start_xpos2", NULL);
	akiset.occ.start_ypos2 = iniparser_getstring( akiset.ini, "occ:start_ypos2", NULL);
	akiset.occ.end_xpos2   = iniparser_getstring( akiset.ini, "occ:end_xpos2", NULL);
	akiset.occ.end_ypos2   = iniparser_getstring( akiset.ini, "occ:end_ypos2", NULL);
	akiset.occ.enable2     = iniparser_getstring( akiset.ini, "occ:enable2", NULL);
	akiset.occ.start_xpos3 = iniparser_getstring( akiset.ini, "occ:start_xpos3", NULL);
	akiset.occ.start_ypos3 = iniparser_getstring( akiset.ini, "occ:start_ypos3", NULL);
	akiset.occ.end_xpos3   = iniparser_getstring( akiset.ini, "occ:end_xpos3", NULL);
	akiset.occ.end_ypos3   = iniparser_getstring( akiset.ini, "occ:end_ypos3", NULL);
	akiset.occ.enable3     = iniparser_getstring( akiset.ini, "occ:enable3", NULL);
	akiset.occ.start_xpos4 = iniparser_getstring( akiset.ini, "occ:start_xpos4", NULL);
	akiset.occ.start_ypos4 = iniparser_getstring( akiset.ini, "occ:start_ypos4", NULL);
	akiset.occ.end_xpos4   = iniparser_getstring( akiset.ini, "occ:end_xpos4", NULL);
	akiset.occ.end_ypos4   = iniparser_getstring( akiset.ini, "occ:end_ypos4", NULL);
	akiset.occ.enable4     = iniparser_getstring( akiset.ini, "occ:enable4", NULL);


	
	nInitaki = 1;
	return nInitaki;
}


struct occ_info * IniSetting_GetOccInfo()
{
	if(!nInitaki)
		return NULL;
	
	akiset.occ.start_xpos1 = iniparser_getstring( akiset.ini, "occ:start_xpos1", NULL);
	akiset.occ.start_ypos1 = iniparser_getstring( akiset.ini, "occ:start_ypos1", NULL);
	akiset.occ.end_xpos1   = iniparser_getstring( akiset.ini, "occ:end_xpos1", NULL);
	akiset.occ.end_ypos1   = iniparser_getstring( akiset.ini, "occ:end_ypos1", NULL);
	akiset.occ.enable1     = iniparser_getstring( akiset.ini, "occ:enable1", NULL);
	akiset.occ.start_xpos2 = iniparser_getstring( akiset.ini, "occ:start_xpos2", NULL);
	akiset.occ.start_ypos2 = iniparser_getstring( akiset.ini, "occ:start_ypos2", NULL);
	akiset.occ.end_xpos2   = iniparser_getstring( akiset.ini, "occ:end_xpos2", NULL);
	akiset.occ.end_ypos2   = iniparser_getstring( akiset.ini, "occ:end_ypos2", NULL);
	akiset.occ.enable2     = iniparser_getstring( akiset.ini, "occ:enable2", NULL);
	akiset.occ.start_xpos3 = iniparser_getstring( akiset.ini, "occ:start_xpos3", NULL);
	akiset.occ.start_ypos3 = iniparser_getstring( akiset.ini, "occ:start_ypos3", NULL);
	akiset.occ.end_xpos3   = iniparser_getstring( akiset.ini, "occ:end_xpos3", NULL);
	akiset.occ.end_ypos3   = iniparser_getstring( akiset.ini, "occ:end_ypos3", NULL);
	akiset.occ.enable3     = iniparser_getstring( akiset.ini, "occ:enable3", NULL);
	akiset.occ.start_xpos4 = iniparser_getstring( akiset.ini, "occ:start_xpos4", NULL);
	akiset.occ.start_ypos4 = iniparser_getstring( akiset.ini, "occ:start_ypos4", NULL);
	akiset.occ.end_xpos4   = iniparser_getstring( akiset.ini, "occ:end_xpos4", NULL);
	akiset.occ.end_ypos4   = iniparser_getstring( akiset.ini, "occ:end_ypos4", NULL);
	akiset.occ.enable4     = iniparser_getstring( akiset.ini, "occ:enable4", NULL);

	
	settingDebug("sstart_xpos1:%s\n", akiset.occ.start_xpos1);
	settingDebug("start_ypos1%s\n", akiset.occ.start_ypos1);
	settingDebug("end_xpos1:%s\n", akiset.occ.end_xpos1);
	settingDebug("end_ypos1:%s\n", akiset.occ.end_ypos1);
	settingDebug("enable1:%s\n", akiset.occ.enable1);


	settingDebug("sstart_xpos2:%s\n", akiset.occ.start_xpos2);
	settingDebug("start_ypos2%s\n", akiset.occ.start_ypos2);
	settingDebug("end_xpos2:%s\n", akiset.occ.end_xpos2);
	settingDebug("end_ypos2:%s\n", akiset.occ.end_ypos2);
	settingDebug("enable2:%s\n", akiset.occ.enable2);
	
	settingDebug("sstart_xpos3:%s\n", akiset.occ.start_xpos3);
	settingDebug("start_ypos3%s\n", akiset.occ.start_ypos3);
	settingDebug("end_xpos3:%s\n", akiset.occ.end_xpos3);
	settingDebug("end_ypos3:%s\n", akiset.occ.end_ypos3);
	settingDebug("enable3:%s\n", akiset.occ.enable3);
	
	settingDebug("sstart_xpos1:%s\n", akiset.occ.start_xpos4);
	settingDebug("start_ypos1%s\n", akiset.occ.start_ypos4);
	settingDebug("end_xpos1:%s\n", akiset.occ.end_xpos4);
	settingDebug("end_ypos1:%s\n", akiset.occ.end_ypos4);
	settingDebug("enable1:%s\n", akiset.occ.enable4);
	
	
	
	
	
	//settingDebug("setting.recoder.time:%s\n", setting.recoder.time);
	
	return &(akiset.occ);
}

struct md_info * IniSetting_GetmdInfo()
{
	if(!nInitaki)
		return NULL;

	akiset.md.matrix_high = iniparser_getstring( akiset.ini, "md:matrix_high", NULL);
	akiset.md.matrix_low = iniparser_getstring( akiset.ini, "md:matrix_low", NULL);
	akiset.md.nSensitivity = iniparser_getstring( akiset.ini, "md:nSensitivity", NULL);
	akiset.md.on = iniparser_getstring( akiset.ini, "md:on", NULL);

	settingDebug("######ssetting.md.matrix:%s\n", akiset.md.matrix_high);
	settingDebug("######ssetting.md.matrix:%s\n", akiset.md.matrix_low);
	settingDebug("######akiset.md.nSensitivity:%s\n", akiset.md.nSensitivity);
	settingDebug("######akiset.md.on:%s\n", akiset.md.on);
	return &(akiset.md);
}

struct isp_info *IniSetting_GetispInfo()
{
	if( !nInitaki )
		return NULL;

	akiset.isp.nContrast = iniparser_getstring( akiset.ini, "isp:nContrast", NULL);
	akiset.isp.nSaturation = iniparser_getstring( akiset.ini, "isp:nSaturation", NULL);
	akiset.isp.nBrightness = iniparser_getstring( akiset.ini, "isp:nBrightness", NULL);

	settingDebug("######akiset.isp.nContrast:%s\n", akiset.isp.nContrast);
	settingDebug("######akiset.isp.nSaturation:%s\n", akiset.isp.nSaturation);
	settingDebug("######akiset.isp.nBrightness:%s\n", akiset.isp.nBrightness);
	return &(akiset.isp);
}

void IniSetting_akidestroy()
{
	if(nInitaki == 1)
	{
		printf("free aki \n");
		iniparser_freedict(akiset.ini);
	}	
	nInitaki = 0;
	return;
}
int IniSetting_akisave()
{
	FILE *fp = NULL;
	
	if(!nInitaki)
		return -2;
	fp = fopen(INI_AKI, "w");
	if (NULL == fp){
		print_error("%s:open %s file fail!", __func__, INI_AKI);
		return -1;
	}

	iniparser_dump_ini(akiset.ini, fp);
	fclose(fp);
	settingDebug("save camera ini file:%s success\n", INI_AKI);
	return 1;
}
