#pragma once
#include "Resource.h"
#include "AWBDialog.h"
#include "BBDialog.h"
#include "BEnhanceDialog.h"
#include "CCorrectDialog.h"
#include "DemosaicDialog.h"
#include "FilterDialog.h"
#include "GammaDialog.h"
#include "LensDialog.h"
#include "SaturationDialog.h"
#include "SpecEffDialog.h"
#include "WBDialog.h"

int GenerateHFileHead(char ** strFileHead);

int GenerateHeadFileEnd(char ** strFileEnd);

int GenerateAWBHFileDefine(AWBINFO * pstAWBInfo, char ** strAWBDefine);

int GenerateBlackBalanceHFileDefine(BLACKBALANCE * pstBB, char ** strBBDefine);

int GenerateBrightnessEnhHFileDefine(BRIENHANCE * pstBriEnhance, char ** strBriEnhDefine);

int GenerateColorCorrectHFileDefine(CCORRECT * pstCC, char ** strCCDefine);

int GenerateDemosaicHFileDefine(DEMOSAIC * pstDemosaic, char ** strDemosaicDefine);

int GenerateRGBFilterHFileDefine(RGBFILTER * pstRGBFilter, char ** strRGBFilterDefine);

int GenerateUVFilterHFileDefine(UVFILTER * pstUVFilter, char ** strUVFilterDefine);

int GenerateDFPDefectHFileDefine(DFPDEFECT * pstDfp, char ** strDfpDefine);

int GenerateGammaHFileDefine(GAMMACALC ** ppstGamma, int nArrayCnt, char ** strGammaDefine);

int GenerateLensHFileDefine(LENSCORRECT * pstLens, char ** strLensDefine);

int GenerateSaturationHFileDefine(SATURATION * pstSaturation, char ** strSaturationDefine);

int GenerateSpecEffectHFileDefine(SPECIALEFF * pstSpecEff, char ** strSpecEffDefine);

int GenerateWhiteBalanceHFileDefine(WBALANCE * pstWB, char ** strWBDefine);
