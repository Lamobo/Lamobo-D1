#include "stdafx.h"
#include "HeadFile.h"

#define _sprintf_ak(strDest, nAllLen, strDestBegin, arg, ...) \
			sprintf_s((strDest), (nAllLen) - strlen(strDestBegin) - 1, ##arg, __VA_ARGS__); \
			(strDest) = (strDestBegin) + strlen(strDestBegin)

#define RETURN_IF_OVERFLOW(curPos, beginPos, nAllLen) \
		if ((curPos) - (beginPos) >= (nAllLen)) { \
			fprintf(stderr, "%s::%s::%s memory overflow!\n\n", __FILE__, __FUNCTION__, __LINE__); \
			delete[] (beginPos); \
			return -1; \
		}

#define MAX_DEFINE_LEN			1024
#define FILE_HEAD_DEFINE		"ISP_INITIALIZE_PARAMETER_DEFINE_H"

#define AW_STRUCT_DEFINE_NAME	"stAwb"
#define BB_STRUCT_DEFINE_NAME	"stBlackBlance"
#define BE_STRUCT_DEFINE_NAME	"stBrigtnessEnhance"
#define CC_STRUCT_DEFINE_NAME	"stColorCorrect"
#define DE_STRUCT_DEFINE_NAME	"stDemosaic"
#define RF_STRUCT_DEFINE_NAME	"stRGBFilter"
#define UF_STRUCT_DEFINE_NAME	"stUVFilter"
#define DF_STRUCT_DEFINE_NAME	"stDefectPixel"
#define GC_STRUCT_DEFINE_NAME	"stGammaCalc"
#define LC_STRUCT_DEFINE_NAME	"stLensCorrect"
#define SD_STRUCT_DEFINE_NAME	"stSaturation"
#define SE_STRUCT_DEFINE_NAME	"stSpecialEffect"
#define WB_STRUCT_DEFINE_NAME	"stWhiteBlance"

char * g_strIncludeFile[] = {"<plat-anyka/isp_interface.h>"};

#define INCLUDE_FILE_CNT (sizeof(g_strIncludeFile) / sizeof(char *))

int GenerateHFileHead(char ** strFileHead)
{
	ASSERT(strFileHead);

	*strFileHead = NULL;

	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));

	strTemp = strDefine;

	//_stprintf(strTemp, "#ifdef %s \n\n", FILE_HEAD_DEFINE);
	//strTemp = strDefine + strlen(strDefine);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "#ifndef %s\n", FILE_HEAD_DEFINE);

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "#define %s\n", FILE_HEAD_DEFINE);
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\n");
	for (int i = 0; i < INCLUDE_FILE_CNT; ++i) {
		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "#include %s\n", g_strIncludeFile[i]);
		if (strTemp - strDefine >= MAX_DEFINE_LEN * sizeof(char)) {
			fprintf(stderr, "%s::%s::%s memory overflow!\n\n", __FILE__, __FUNCTION__, __LINE__);
			delete[] strDefine;
			return -1;
		}
	}
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\n");
	
	*strFileHead = strDefine;
	return 0;
}

int GenerateHeadFileEnd(char ** strFileEnd)
{
	ASSERT(strFileEnd);

	*strFileEnd = NULL;

	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));

	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "#endif\n");

	*strFileEnd = strDefine;
	return 0;
}

int GenerateAWBHFileDefine(AWBINFO * pstAWBInfo, char ** strAWBDefine)
{
	ASSERT(pstAWBInfo && strAWBDefine && (pstAWBInfo->type == ISP_CID_AUTO_WHITE_BALANCE));
	
	*strAWBDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_auto_white_balance %s[] = {\n", AW_STRUCT_DEFINE_NAME);
	for(int i=0; i<5; i++)
	{
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "{\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", (pstAWBInfo+i)->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", (pstAWBInfo+i)->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", (pstAWBInfo+i)->index);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->gr_low);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->gr_high);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->gb_low);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->gb_high);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->grb_low);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->grb_high);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->r_low);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->r_high);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->g_low);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->g_high);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->b_low);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->b_high);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->co_r);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", (pstAWBInfo+i)->co_g);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u\n", (pstAWBInfo+i)->co_b);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	if(i < 4)
	{
		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, " },\n");
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	}
	else
	{
		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, " }\n");
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	}
		
	}

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	*strAWBDefine = strDefine;
	return 0;
}

int GenerateBlackBalanceHFileDefine(BLACKBALANCE * pstBB, char ** strBBDefine)
{
	ASSERT(pstBB && strBBDefine && (pstBB->type == ISP_CID_BLACK_BALANCE));
	
	*strBBDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_black_balance %s = {\n", BB_STRUCT_DEFINE_NAME);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", pstBB->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstBB->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstBB->r_offset);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstBB->g_offset);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u\n", pstBB->b_offset);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	*strBBDefine = strDefine;
	return 0;
}

int GenerateBrightnessEnhHFileDefine(BRIENHANCE * pstBriEnhance, char ** strBriEnhDefine)
{
	ASSERT(pstBriEnhance && strBriEnhDefine && (pstBriEnhance->type == ISP_CID_BRIGHTNESS_ENHANCE));
	
	*strBriEnhDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_brightness_enhance %s = {\n", BE_STRUCT_DEFINE_NAME);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", pstBriEnhance->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstBriEnhance->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstBriEnhance->ygain);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstBriEnhance->y_thrs);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u\n", pstBriEnhance->y_edgek);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	*strBriEnhDefine = strDefine;
	return 0;
}

int GenerateColorCorrectHFileDefine(CCORRECT * pstCC, char ** strCCDefine)
{
	ASSERT(pstCC && strCCDefine && (pstCC->type == ISP_CID_COLOR));
	
	*strCCDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_color_correct %s = {\n", CC_STRUCT_DEFINE_NAME);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", pstCC->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstCC->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstCC->cc_thrs_low);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstCC->cc_thrs_high);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t{\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	for (int i = 0; i < 3; ++i) {
		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t\t{");
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

		for (int j = 0; j < 3; ++j) {
			_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "%d", pstCC->ccMtrx[i][j]);
			RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
			if (j < 2) {
				_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, ", ");
				RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
			}
		}

		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "},\n");
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	}
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t}\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	*strCCDefine = strDefine;
	return 0;
}

int GenerateDemosaicHFileDefine(DEMOSAIC * pstDemosaic, char ** strDemosaicDefine)
{
	ASSERT(pstDemosaic && strDemosaicDefine && (pstDemosaic->type == ISP_CID_DEMOSAIC));
	
	*strDemosaicDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_demosaic %s = {\n", DE_STRUCT_DEFINE_NAME);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", pstDemosaic->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstDemosaic->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u\n", pstDemosaic->threshold);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	*strDemosaicDefine = strDefine;
	return 0;
}

int GenerateRGBFilterHFileDefine(RGBFILTER * pstRGBFilter, char ** strRGBFilterDefine)
{
	ASSERT(pstRGBFilter && strRGBFilterDefine && (pstRGBFilter->type == ISP_CID_RGB_FILTER));
	
	*strRGBFilterDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_rgb_filter %s = {\n", RF_STRUCT_DEFINE_NAME);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", pstRGBFilter->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstRGBFilter->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u\n", pstRGBFilter->threshold);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	*strRGBFilterDefine = strDefine;
	return 0;
}

int GenerateUVFilterHFileDefine(UVFILTER * pstUVFilter, char ** strUVFilterDefine)
{
	ASSERT(pstUVFilter && strUVFilterDefine && (pstUVFilter->type == ISP_CID_UV_FILTER));
	
	*strUVFilterDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_uv_filter %s = {\n", UF_STRUCT_DEFINE_NAME);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", pstUVFilter->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d\n", pstUVFilter->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	*strUVFilterDefine = strDefine;
	return 0;
}

int GenerateDFPDefectHFileDefine(DFPDEFECT * pstDfp, char ** strDfpDefine)
{
	ASSERT(pstDfp && strDfpDefine && (pstDfp->type == ISP_CID_DEFECT_PIXEL));
	
	*strDfpDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_defect_pixel %s = {\n", DF_STRUCT_DEFINE_NAME);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", pstDfp->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstDfp->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u\n", pstDfp->threshold);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	*strDfpDefine = strDefine;
	return 0;
}

int GenerateGammaHFileDefine(GAMMACALC ** ppstGamma, int nArrayCnt, char ** strGammaDefine)
{
	ASSERT(ppstGamma && strGammaDefine);
	
	*strGammaDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN * 2];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_gamma_calculate %s[] = {\n", GC_STRUCT_DEFINE_NAME);

	for (int i = 0; i < nArrayCnt; ++i) {
		ASSERT(ppstGamma[i] && (ppstGamma[i]->type == ISP_CID_GAMMA));
		
		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t{\n");
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t\t0x%x,\n", ppstGamma[i]->type);
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t\t%d,\n", ppstGamma[i]->enable);
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t\t%d,\n", ppstGamma[i]->sync);
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
		
		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t\t{\n");
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t\t\t");
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
		for (int j = 1; j <= 32; ++j) {
			_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "0x%x", ppstGamma[i]->gamma[j - 1]);

			if (j < 32) {
				_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, ", ");
				RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
			}else {
				_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\n");
				RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
			}

			if ((j != 32) && !(j % 8)) {
				_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\n\t\t\t");
				RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
			}
		}
		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t\t}\n");
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t}");
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

		if (i < 1) {
			_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, ",\n");
			RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
		}else {
			_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\n");
			RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));			
		}
	}

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	*strGammaDefine = strDefine;
	return 0;
}

static int GenerateLensArrayDefine(char * strTemp, char * strDefine, int * piArray, int iArrayCnt)
{
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t{\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t\t");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	for (int i = 1; i <= iArrayCnt; ++i) {
		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "%u", piArray[i - 1]);
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

		if (i < iArrayCnt) {
			_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, ", ");
			RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
		}else {
			_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\n");
			RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
		}

		if ((i != iArrayCnt) && !(i % 5)) {
			_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\n\t\t");
			RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));		
		}
	}
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t},\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	return strTemp - strDefine; 
}

static int GenerateLensArrayDefineUnsigned(char * strTemp, char * strDefine, unsigned int * piArray, int iArrayCnt)
{
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t{\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t\t");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	for (int i = 1; i <= iArrayCnt; ++i) {
		_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "%u", piArray[i - 1]);
		RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

		if (i < iArrayCnt) {
			_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, ", ");
			RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
		}else {
			_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\n");
			RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
		}

		if ((i != iArrayCnt) && !(i % 5)) {
			_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\n\t\t");
			RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));		
		}
	}
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t},\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	return strTemp - strDefine; 
}

int GenerateLensHFileDefine(LENSCORRECT * pstLens, char ** strLensDefine)
{
	ASSERT(pstLens && strLensDefine && (pstLens->type == ISP_CID_LENS));
	
	*strLensDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_lens_correct %s = {\n", LC_STRUCT_DEFINE_NAME);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", pstLens->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstLens->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	int ret = 0;
	if ((ret = GenerateLensArrayDefine(strTemp, strDefine, pstLens->lens_coefa, 10)) < 0)
		return -1;

	strTemp = strDefine + ret;
	
	ret = 0;
	if ((ret = GenerateLensArrayDefine(strTemp, strDefine, pstLens->lens_coefb, 10)) < 0)
		return -1;

	strTemp = strDefine + ret;
	
	ret = 0;
	if ((ret = GenerateLensArrayDefine(strTemp, strDefine, pstLens->lens_coefc, 10)) < 0)
		return -1;

	strTemp = strDefine + ret;

	ret = 0;
	if ((ret = GenerateLensArrayDefineUnsigned(strTemp, strDefine, pstLens->lens_range, 10)) < 0)
		return -1;

	strTemp = strDefine + ret;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstLens->lens_xref);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u\n", pstLens->lens_yref);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	*strLensDefine = strDefine;
	return 0;
}

int GenerateSaturationHFileDefine(SATURATION * pstSaturation, char ** strSaturationDefine)
{
	ASSERT(pstSaturation && strSaturationDefine && (pstSaturation->type == ISP_CID_SATURATION));
	
	*strSaturationDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_saturation %s = {\n", SD_STRUCT_DEFINE_NAME);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", pstSaturation->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstSaturation->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstSaturation->Khigh);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstSaturation->Klow);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstSaturation->Kslope);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstSaturation->Chigh);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u\n", pstSaturation->Clow);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	*strSaturationDefine = strDefine;
	return 0;
}

int GenerateSpecEffectHFileDefine(SPECIALEFF * pstSpecEff, char ** strSpecEffDefine)
{
	ASSERT(pstSpecEff && strSpecEffDefine && (pstSpecEff->type == ISP_CID_SPECIAL_EFFECT));
	
	*strSpecEffDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_special_effect %s = {\n", SE_STRUCT_DEFINE_NAME);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", pstSpecEff->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstSpecEff->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstSpecEff->solar_enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstSpecEff->solar_thrs);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstSpecEff->y_eff_coefa);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstSpecEff->y_eff_coefb);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstSpecEff->u_eff_coefa);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstSpecEff->u_eff_coefb);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstSpecEff->v_eff_coefa);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u\n", pstSpecEff->v_eff_coefb);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	*strSpecEffDefine = strDefine;
	return 0;
}

int GenerateWhiteBalanceHFileDefine(WBALANCE * pstWB, char ** strWBDefine)
{
	ASSERT(pstWB && strWBDefine && (pstWB->type == ISP_CID_WHITE_BALANCE));
	
	*strWBDefine = NULL;
	
	char * strTemp = NULL;
	char * strDefine = new char[MAX_DEFINE_LEN];
	if (strDefine == NULL) {
		fprintf(stderr, "%s::%::%s out of memory!\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(strDefine, 0, MAX_DEFINE_LEN * sizeof(char));
	
	strTemp = strDefine;

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "struct isp_white_balance %s = {\n", WB_STRUCT_DEFINE_NAME);
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t0x%x,\n", pstWB->type);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%d,\n", pstWB->enable);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstWB->co_r);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u,\n", pstWB->co_g);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));

	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "\t%u\n", pstWB->co_b);
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	_sprintf_ak(strTemp, MAX_DEFINE_LEN, strDefine, "};\n\n");
	RETURN_IF_OVERFLOW(strTemp, strDefine, MAX_DEFINE_LEN * sizeof(char));
	
	*strWBDefine = strDefine;
	return 0;
}