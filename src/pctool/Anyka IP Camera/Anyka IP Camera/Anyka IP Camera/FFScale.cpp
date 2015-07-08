#include "StdAfx.h"
#include "FFScale.h"

//构造
CFFScale::CFFScale(void)
{
	m_pSwsContext = NULL;
	m_srcFormat = SWS_PF_NONE;	
	m_dstFormat = SWS_PF_NONE;	
	m_enAlogrithm = SWS_SA_FAST_BILINEAR;

	m_nSrcW = m_nSrcH = 0;			
	m_nSrcPicth = 0;				
	m_nDstW = m_nDstH = 0;
	m_nDstPicth = 0;
	for (int i=0; i<4; i++)
	{
		m_nSrcSlice[i] = -1;
		m_nSrcStride[i] = 0;
		m_nDstSlice[i] = -1;
		m_nDstStride[i] = 0; 
	}
}

//析构
CFFScale::~CFFScale(void)
{
	DeInit();
}

/***************************************************************************//**
* 函数名称：	SetAttribute
* 功能描述：	设置输入输出图片属性以及Scale算法。
* 参    数：	srcFormat	>> 源图像格式；
* 参    数：	dstFormat	>> 目标图像格式；
* 参    数：	enAlogrithm	>> Scale算法；
* 返 回 值：	
* 其它说明：	
* 修改日期		修改人			修改内容
* ------------------------------------------------------------------------------
* 2011-10-28	Cloud	      	创建
*******************************************************************************/
void CFFScale::SetAttribute(PicFormat srcFormat, PicFormat dstFormat, SwsAlogrithm enAlogrithm)
{
	m_srcFormat = srcFormat;
	m_dstFormat = dstFormat;
	m_enAlogrithm = enAlogrithm;
	DeInit();
}

/***************************************************************************//**
* 函数名称：	Init
* 功能描述：	初始化。
* 返 回 值：	执行成功返回TRUE，否则返回FALSE。
* 其它说明：	
* 修改日期		修改人			修改内容
* ------------------------------------------------------------------------------
* 2011-10-28	Cloud	      	创建
*******************************************************************************/
BOOL CFFScale::Init()
{
	//必须预先设置过输入输出格式
	if (SWS_PF_NONE == m_srcFormat || SWS_PF_NONE == m_dstFormat)
	{
		return FALSE;
	}

	//反初始化
	DeInit();

	//创建sws对象
	m_pSwsContext = sws_getContext(
		m_nSrcW,
		m_nSrcH,
		(PixelFormat)m_srcFormat,
		m_nDstW,
		m_nDstH,
		(PixelFormat)m_dstFormat,
		(int)m_enAlogrithm,
		NULL, 
		NULL, 
		NULL);
	if (NULL == m_pSwsContext)
	{
		return FALSE;
	}

	//初始化源Slice和Stride
	if (m_srcFormat == SWS_PF_YUV420P)
	{
		m_nSrcSlice[0] = 0;
		m_nSrcSlice[1] = m_nSrcW * m_nSrcH;
		m_nSrcSlice[2] = m_nSrcW * m_nSrcH * 5 / 4;
		m_nSrcSlice[3] = -1;

		m_nSrcStride[0] = m_nSrcW;
		m_nSrcStride[1] = m_nSrcW / 2;
		m_nSrcStride[2] = m_nSrcW / 2;
		m_nSrcStride[3] = 0;

	}
	else
	{
		m_nSrcSlice[0] = 0;
		m_nSrcSlice[1] = -1;
		m_nSrcSlice[2] = -1;
		m_nSrcSlice[3] = -1;

		m_nSrcStride[0] = m_nSrcPicth;
		m_nSrcStride[1] = 0;
		m_nSrcStride[2] = 0;
		m_nSrcStride[3] = 0;
	}

	//初始化目标Slice和Stride
	if (m_dstFormat == SWS_PF_YUV420P)
	{
		m_nDstSlice[0] = 0;
		m_nDstSlice[1] = m_nDstW * m_nDstH;
		m_nDstSlice[2] = m_nDstW * m_nDstH * 5 / 4;
		m_nDstSlice[3] = -1;

		m_nDstStride[0] = m_nDstW;
		m_nDstStride[1] = m_nDstW / 2;
		m_nDstStride[2] = m_nDstW / 2;
		m_nDstStride[3] = 0;

	}
	else
	{
		m_nDstSlice[0] = 0;
		m_nDstSlice[1] = -1;
		m_nDstSlice[2] = -1;
		m_nDstSlice[3] = -1;

		m_nDstStride[0] = m_nDstPicth;
		m_nDstStride[1] = 0;
		m_nDstStride[2] = 0;
		m_nDstStride[3] = 0;
	}
	return TRUE;
}

/***************************************************************************//**
* 函数名称：	DeInit
* 功能描述：	反初始化。
* 返 回 值：	
* 其它说明：	
* 修改日期		修改人			修改内容
* ------------------------------------------------------------------------------
* 2011-10-28	Cloud	      	创建
*******************************************************************************/
void CFFScale::DeInit()
{
	if (NULL != m_pSwsContext)
	{
		sws_freeContext(m_pSwsContext);
	}
	m_pSwsContext = NULL;
}

/***************************************************************************//**
* 函数名称：	Scale
* 功能描述：	Scale
* 参    数：	pSrc			>> 源图像内存起始地址；
* 参    数：	nSrcW			>> 源图像宽度；
* 参    数：	nSrcH			>> 源图像高度；
* 参    数：	nSrcPicth		>> 源图像每行数据的长度（YUV格式的该值不被采纳）；
* 参    数：	pDst			<< 目标图像内存起始地址；
* 参    数：	nDstW			>> 目标图像宽度；
* 参    数：	nDstH			>> 目标图像高度；
* 参    数：	nDstPicth		>> 目标图像每行数据的长度（YUV格式的该值不被采纳）；
* 返 回 值：	执行成功返回TRUE，否则返回FALSE。
* 其它说明：	
* 修改日期		修改人			修改内容
* ------------------------------------------------------------------------------
* 2011-10-28	Cloud	      	创建
*******************************************************************************/
BOOL CFFScale::Scale(byte *pSrc, int nSrcW, int nSrcH, int nSrcPicth, byte *pDst, int nDstW, int nDstH, int nDstPicth)
{
	//如果任何参数发生变化，则需要重新初始化
	if (nSrcW != m_nSrcW || nSrcH != m_nSrcH || nSrcPicth != m_nSrcPicth
		|| nDstW != m_nDstW || nDstH != m_nDstH || nDstPicth != m_nDstPicth)
	{
		m_nSrcW = nSrcW;
		m_nSrcH = nSrcH;
		m_nSrcPicth = nSrcPicth;
		m_nDstW = nDstW;
		m_nDstH = nDstH;
		m_nDstPicth = nDstPicth;
		DeInit();
	}

	//如果未能成功初始化，返回失败
	if (NULL == m_pSwsContext && !Init())
	{
		return FALSE;
	}

	//真正的Scale操作
	byte *srcSlice[4], *dstSlice[4];
	for (int i=0; i<4; i++)
	{
		srcSlice[i] = m_nSrcSlice[i] < 0 ? NULL : (pSrc + m_nSrcSlice[i]);
		dstSlice[i] = m_nDstSlice[i] < 0 ? NULL : (pDst + m_nDstSlice[i]);
	}
	return sws_scale
		(
		m_pSwsContext,
		srcSlice,
		m_nSrcStride,
		0,
		m_nSrcH,
		dstSlice,
		m_nDstStride
		) == m_nSrcH;
}
