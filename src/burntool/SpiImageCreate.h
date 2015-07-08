// SpiImageCreate.h: interface for the SpiImageCreate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPIIMAGECREATE_H__0474AD1B_8092_4CB7_88A4_AAF5B395D919__INCLUDED_)
#define AFX_SPIIMAGECREATE_H__0474AD1B_8092_4CB7_88A4_AAF5B395D919__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern "C"
{
	#include "fha.h"
	#include "fha_asa.h"
}

class SpiImageCreate  
{
public:
	SpiImageCreate();
	virtual ~SpiImageCreate();

public:
	void SpiImageCreate::Spi_Enter(CString spiName, CString strPath);
};

#endif // !defined(AFX_SPIIMAGECREATE_H__0474AD1B_8092_4CB7_88A4_AAF5B395D919__INCLUDED_)
