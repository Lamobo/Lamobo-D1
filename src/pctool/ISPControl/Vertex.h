#pragma once

#include "d3dx9.h"

struct VERTEX
{
	D3DXVECTOR3 pos;        // vertex untransformed position
	D3DCOLOR    color;      // diffuse color
	D3DXVECTOR2 texPos;     // texture relative coordinates
};

#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 )