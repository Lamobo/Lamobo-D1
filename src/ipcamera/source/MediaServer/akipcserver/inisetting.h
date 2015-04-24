

#ifdef __cplusplus
extern "C" {
#endif


#include "iniparser.h"
#include "cgi_anyka.h"

int IniSetting_init();
void IniSetting_destroy();
int IniSetting_save();

struct system_info* IniSetting_GetSysInfo();
struct user_info* IniSetting_GetUsrInfo();
struct recoder_info* IniSetting_GetRecInfo();
struct wireless_info* IniSetting_GetWilInfo();
struct ethernet_info* IniSetting_GetEthInfo();
struct global_info* IniSetting_GetGloInfo();
struct video_info * IniSetting_GetVideoInfo();

struct recoder_info * IniSetting_GetRecordInfo();
struct picture_info *IniSetting_GetPictureInfo();


int IniSetting_SetSysInfo(struct system_info* sysinfo);
int IniSetting_SetUsrInfo(struct user_info* userinfo);
int IniSetting_SetRecInfo(struct recoder_info* recinfo);
int IniSetting_SetWilInfo(struct wireless_info* wilinfo);
int IniSetting_SetEthInfo(struct ethernet_info* ethinfo);
int IniSetting_SetGloInfo(struct global_info* gloinfo);
int IniSetting_aki();
struct occ_info * IniSetting_GetOccInfo();
void IniSetting_akidestroy();
int IniSetting_akisave();
struct md_info * IniSetting_GetmdInfo();
struct isp_info *IniSetting_GetispInfo();




#ifdef __cplusplus
} /* end extern "C" */
#endif

