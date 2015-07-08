#pragma once

#include "d3d9.h"
#include <windows.h>
#include <map>
#include "Macros.h"
#include "AutoLock.h"

#pragma warning(disable:4244)

using namespace std;

class Overlay
{
public:
	Overlay(IDirect3DDevice9* device, D3DCOLOR color, BYTE opacity)
	{
		m_device = device;
		m_opacity = opacity;
		m_color = (color & 0x00FFFFFF) | (opacity << 24);
	}

	virtual ~Overlay(void)
	{

	}

	virtual HRESULT Draw(void) = 0;

protected:
	IDirect3DDevice9* m_device;
	D3DCOLOR m_color;
	BYTE m_opacity;
};

class LineOverlay : public Overlay
{
public:
	LineOverlay(IDirect3DDevice9* device, POINT p1, POINT p2, INT width, D3DCOLOR color, BYTE opacity)
		: Overlay(device, color, opacity)
	{
		HRESULT hr = D3DXCreateLine(m_device, &m_line);
		m_vectors[0].x = p1.x;
		m_vectors[0].y = p1.y;
		m_vectors[1].x = p2.x;
		m_vectors[1].y = p2.y;
		m_line->SetWidth(width);
	}

	virtual ~LineOverlay()
	{
		SafeRelease(m_line);
	}

	virtual HRESULT Draw(void)
	{
		HR(m_line->Begin());
		HR(m_line->Draw(m_vectors, 2, m_color));
		return m_line->End();
	}

private:
	CComPtr<ID3DXLine> m_line;
	D3DXVECTOR2 m_vectors[2];
};

class RectangleOverlay : public Overlay
{
public:
	RectangleOverlay(IDirect3DDevice9* device, RECT rectangle, INT width, D3DCOLOR color, BYTE opacity)
		: Overlay(device, color, opacity)
	{
		D3DXCreateLine(m_device, &m_line);
		m_line->SetWidth(width);

		m_vectors[0].x = rectangle.left;
		m_vectors[0].y = rectangle.top;
		m_vectors[1].x = rectangle.right;
		m_vectors[1].y = rectangle.top;
		m_vectors[2].x = rectangle.right;
		m_vectors[2].y = rectangle.bottom;
		m_vectors[3].x = rectangle.left;
		m_vectors[3].y = rectangle.bottom;
		m_vectors[4].x = rectangle.left;
		m_vectors[4].y = rectangle.top;
	}

	virtual ~RectangleOverlay()
	{
		SafeRelease(m_line);
	}

	virtual HRESULT Draw(void)
	{
		HR(m_line->Begin());
		HR(m_line->Draw(m_vectors, 5, m_color));
		return m_line->End();
	}

private:
	CComPtr<ID3DXLine> m_line;
	D3DXVECTOR2 m_vectors[5];
};

class PolygonOverlay : public Overlay
{
public:
	PolygonOverlay(IDirect3DDevice9* device, POINT* points, INT pointsLen, INT width, D3DCOLOR color, BYTE opacity)
		: Overlay(device, color, opacity) 
	{
		HRESULT hr = D3DXCreateLine(m_device, &m_line);
		m_vectors = new D3DXVECTOR2[pointsLen + 1];
		for(int i = 0 ; i < pointsLen; i++)
		{
			m_vectors[i].x = points[i].x;
			m_vectors[i].y = points[i].y;
		}

		m_vectors[pointsLen].x = points[0].x;
		m_vectors[pointsLen].y = points[0].y;

		m_numOfVectors = pointsLen + 1;
		m_line->SetWidth(width);
	}

	virtual ~PolygonOverlay()
	{
		delete m_vectors;
		SafeRelease(m_line);
	}

	virtual HRESULT Draw(void)
	{
		HR(m_line->Begin());
		HR(m_line->Draw(m_vectors, m_numOfVectors, m_color));
		return m_line->End();
	}

private:
	CComPtr<ID3DXLine> m_line;
	D3DXVECTOR2* m_vectors;
	INT m_numOfVectors;
};

class D3DFont
{
public:
	D3DFont(IDirect3DDevice9* device, LPCWSTR fontName, INT fontSize, UINT weight)
	{
		D3DXCreateFont( device, fontSize, 0, weight, 0, FALSE, 0, 0, 0, 0, fontName, &m_font );
	}

	virtual ~D3DFont()
	{
		SafeRelease(m_font);
	}

	HRESULT PrepareText(LPCWSTR pText)
	{
		return m_font->PreloadText(pText, -1);
	}

	HRESULT DrawText(LPCWSTR pText, LPRECT rect, D3DCOLOR color)
	{
		return m_font->DrawText(NULL, pText, -1, rect, 0, color);
	}

private:
	CComPtr<ID3DXFont> m_font;
};

class TextOverlay : public Overlay
{
public:
	TextOverlay(IDirect3DDevice9* device, LPCWSTR text, RECT pos, INT size, D3DCOLOR color, LPCWSTR font, BYTE opacity)
		: Overlay(device, color, opacity)
	{
		m_pos = pos;
		int len = wcslen(text);
		m_text = new WCHAR[len + 1];
		wmemcpy(m_text, text, len);
		m_text[len] = '\0';

		pFont = new D3DFont(device, font, size, FW_NORMAL);
		pFont->PrepareText(m_text);
	}

	virtual ~TextOverlay()
	{
		delete pFont;
		delete m_text;
	}

	virtual HRESULT Draw(void)
	{
		return pFont->DrawText(m_text, &m_pos, m_color );
	}

private:
	WCHAR* m_text;
	RECT m_pos;
	D3DFont* pFont;
};

class BaseBitmapOverlay : public Overlay
{
public:
	BaseBitmapOverlay(IDirect3DDevice9* device, RECT rectangle, BYTE opacity)
		: Overlay(device, D3DCOLOR_ARGB(0xff, 0, 0, 0), opacity), m_applyShaders(false)
	{
		m_rectangle = rectangle;
		HRESULT hr = m_device->CreateVertexBuffer(sizeof(VERTEX) * 4, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL);	
	}

	virtual ~BaseBitmapOverlay()
	{
		SafeRelease(m_pVertexBuffer);
		SafeRelease(m_pTexture);
	}

	virtual HRESULT Draw(void)
	{
		HRESULT hr = S_OK;

		VERTEX vertexArray[] =
		{
			{ D3DXVECTOR3(m_rectangle.left, m_rectangle.top, 0),     D3DCOLOR_ARGB(m_opacity, 255, 255, 255), D3DXVECTOR2(0, 0) },  // top left
			{ D3DXVECTOR3(m_rectangle.right, m_rectangle.top, 0),    D3DCOLOR_ARGB(m_opacity, 255, 255, 255), D3DXVECTOR2(1, 0) },  // top right
			{ D3DXVECTOR3(m_rectangle.right, m_rectangle.bottom, 0), D3DCOLOR_ARGB(m_opacity, 255, 255, 255), D3DXVECTOR2(1, 1) },  // bottom right
			{ D3DXVECTOR3(m_rectangle.left, m_rectangle.bottom, 0),  D3DCOLOR_ARGB(m_opacity, 255, 255, 255), D3DXVECTOR2(0, 1) },  // bottom left
		};

		VERTEX *vertices;
		hr = m_pVertexBuffer->Lock(0, 0, (void**)&vertices, D3DLOCK_DISCARD);
		memcpy(vertices, vertexArray, sizeof(vertexArray));
		hr = m_pVertexBuffer->Unlock();	

		if(!m_applyShaders)
		{
			m_device->SetPixelShader(NULL);
			m_device->SetVertexShader(NULL);
		}
		hr = m_device->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(VERTEX));
		hr = m_device->SetFVF(D3DFVF_CUSTOMVERTEX);
		hr = m_device->SetTexture(0, m_pTexture);	
		hr = m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
		hr = m_device->SetTexture(0, NULL);	

		return hr;
	}

	virtual HRESULT Update(BYTE* pYplane, BYTE* pVplane, BYTE* pUplane)
	{
		return S_OK;
	}

	void EnablePixelShaders(bool bEnable)
	{
		m_applyShaders = bEnable;
	}

	void UpdateOpacity(BYTE opacity)
	{
		m_opacity = opacity;
	}

protected:
	CComPtr<IDirect3DTexture9> m_pTexture;
	bool m_applyShaders;
	RECT m_rectangle;
private:	
	CComPtr<IDirect3DVertexBuffer9> m_pVertexBuffer;
};

class FileOverlay : public BaseBitmapOverlay
{
public:
	FileOverlay(IDirect3DDevice9* device, RECT rectangle, LPCWSTR path, D3DCOLOR colorKey, BYTE opacity)
		: BaseBitmapOverlay(device, rectangle, opacity)
	{
		HRESULT hr = D3DXCreateTextureFromFileEx(m_device, 
							path,
						    D3DX_DEFAULT,    // default width
                            D3DX_DEFAULT,    // default height
                            D3DX_DEFAULT,    // no mip mapping
                            NULL,                // regular usage
                            D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                            D3DPOOL_MANAGED,    // typical memory handling
                            D3DX_DEFAULT,    // no filtering
                            D3DX_DEFAULT,    // no mip filtering
                            colorKey,    // color key
                            NULL,    // no image info struct
                            NULL,    // not using 256 colors
                            &m_pTexture);    // load to texture	

		if(FAILED(hr))
		{
			char msg[200];
			sprintf_s(msg, "Failed to load texture from file, error %d", hr);
			throw msg;
		}
	}
};

class MemoryBitmapOverlay : public BaseBitmapOverlay
{
public:
	MemoryBitmapOverlay(IDirect3DDevice9* device, RECT rectangle, BYTE* pixelData, UINT stride, INT width, INT height, D3DCOLOR colorKey, BYTE opacity)
		: BaseBitmapOverlay(device, rectangle, opacity)
	{
		CComPtr<IDirect3DSurface9> surface;
		HRESULT hr = m_device->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &surface, NULL);
		
		D3DLOCKED_RECT rect;
		hr = surface->LockRect(&rect, NULL, 0);
		
		for (int z = 0; z < height; z++)
		{
			DWORD* pSource = (DWORD*)pixelData + z * stride / 4;
			DWORD* pTarget = (DWORD*)rect.pBits + z * rect.Pitch / 4;

			for (int x = 0; x < width; x++)
			{		
				pTarget[x] = pSource[x];
				if(pSource[x] == colorKey)
				{
					pTarget[x] &= 0x00FFFFFF;
				}
			}
		}

		hr = surface->UnlockRect();

		int w = rectangle.right - rectangle.left;
		int h = rectangle.bottom - rectangle.top;
		hr = m_device->CreateTexture(w, h, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexture, NULL);

		CComPtr<IDirect3DSurface9> pTextureSurface;
		hr = m_pTexture->GetSurfaceLevel(0, &pTextureSurface);
		hr = m_device->ColorFill(pTextureSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0));
		hr = m_device->StretchRect(surface, NULL, pTextureSurface, NULL, D3DTEXF_LINEAR);
	}
};

class VideoOverlay : public BaseBitmapOverlay
{
public:
	VideoOverlay(IDirect3DDevice9* device, int videoWidth, int videoHeight, D3DFORMAT videoFormat, RECT targetRect, BYTE opacity)
		: BaseBitmapOverlay(device, targetRect, opacity)
	{
		HRESULT hr = m_device->CreateOffscreenPlainSurface(videoWidth, videoHeight, videoFormat, D3DPOOL_DEFAULT, &m_pSurface, NULL);
		m_width = videoWidth;
		m_height = videoHeight;
		m_format = videoFormat;

		int w = targetRect.right - targetRect.left;
		int h = targetRect.bottom - targetRect.top;
		hr = m_device->CreateTexture(w, h, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pTexture, NULL);
		hr = m_pTexture->GetSurfaceLevel(0, &m_pTextureSurface);
		hr = m_device->ColorFill(m_pTextureSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0));
	}

	virtual HRESULT Update(BYTE* pYplane, BYTE* pVplane, BYTE* pUplane)
	{
		int newHeight  = m_height;
		int newWidth  = m_width;

		D3DLOCKED_RECT d3drect;
		HR(m_pSurface->LockRect(&d3drect, NULL, 0));

		BYTE* pict = (BYTE*)d3drect.pBits;
		BYTE* Y = pYplane;
		BYTE* V = pVplane;
		BYTE* U = pUplane;

		switch(m_format)
		{
		case D3DFMT_YV12:

			for (int y = 0 ; y < newHeight ; y++)
			{
				memcpy(pict, Y, newWidth);
				pict += d3drect.Pitch;
				Y += newWidth;
			}
			for (int y = 0 ; y < newHeight >> 1 ; y++)
			{
				memcpy(pict, V, newWidth >> 1);
				pict += d3drect.Pitch >> 1;
				V += newWidth >> 1;
			}
			for (int y = 0 ; y < newHeight >> 1; y++)
			{
				memcpy(pict, U, newWidth >> 1);
				pict += d3drect.Pitch >> 1;
				U += newWidth >> 1;
			}	
			break;

		case D3DFMT_NV12:

			for (int y = 0 ; y < newHeight ; y++)
			{
				memcpy(pict, Y, newWidth);
				pict += d3drect.Pitch;
				Y += newWidth;
			}
			for (int y = 0 ; y < newHeight >> 1 ; y++)
			{
				memcpy(pict, V, newWidth);
				pict += d3drect.Pitch;
				V += newWidth;
			}
			break;

		case D3DFMT_YUY2:
		case D3DFMT_UYVY:
		case D3DFMT_R5G6B5:
		case D3DFMT_X1R5G5B5:
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:

			memcpy(pict, Y, d3drect.Pitch * newHeight);

			break;
		}
			
		m_pSurface->UnlockRect();

		return m_device->StretchRect(m_pSurface, NULL, m_pTextureSurface, NULL, D3DTEXF_LINEAR);
	}

	~VideoOverlay()
	{
		SafeRelease(m_pTextureSurface);
		SafeRelease(m_pSurface);
	}

private:
	CComPtr<IDirect3DSurface9> m_pSurface;
	CComPtr<IDirect3DSurface9> m_pTextureSurface;
	int m_width;
	int m_height;
	D3DFORMAT m_format;
};

class OverlayStore
{
	typedef map<SHORT, Overlay*> OverlayMap;

public:
	OverlayStore()
	{
	}

	virtual ~OverlayStore()
	{
		RemoveAll();
	}

	void AddOverlay(Overlay* pOverlay, SHORT id)
	{
		CAutoLock lock(&m_lock);
		OverlayMap::iterator i = m_overlays.find(id);
		if (i != m_overlays.end())
		{
			delete i->second;
		}
		m_overlays[id] = pOverlay;
	}

	void UpdateOverlay(SHORT key, BYTE* pYplane, BYTE* pVplane, BYTE* pUplane)
	{
		CAutoLock lock(&m_lock);
		OverlayMap::iterator i = m_overlays.find(key);
		if (i != m_overlays.end())
		{
			BaseBitmapOverlay* baseBitamp = dynamic_cast<BaseBitmapOverlay*>(i->second);
			baseBitamp->Update(pYplane, pVplane, pUplane);
		}
	}

	void RemoveOverlay(SHORT id)
	{
		CAutoLock lock(&m_lock);
		OverlayMap::iterator i = m_overlays.find(id);
		if (i != m_overlays.end())
		{
			delete i->second;
			m_overlays.erase(i);
		}
	}

	void Draw()
	{
		if(IsEmpty())
		{
			return;
		}

		CAutoLock lock(&m_lock);
		for each(pair<SHORT, Overlay*> pair in m_overlays )
		{
			pair.second->Draw();
		}
	}

	bool IsEmpty()
	{
		CAutoLock lock(&m_lock);
		return m_overlays.empty();
	}

	void RemoveAll()
	{
		CAutoLock lock(&m_lock);
		for (OverlayMap::iterator i = m_overlays.begin(); i != m_overlays.end(); ++i)
		{
			delete i->second;
		}
		m_overlays.clear();
	}

	void UpdateOverlayOpacity( SHORT key, BYTE opacity ) 
	{
		CAutoLock lock(&m_lock);
		OverlayMap::iterator i = m_overlays.find(key);
		if (i != m_overlays.end())
		{
			BaseBitmapOverlay* baseBitamp = dynamic_cast<BaseBitmapOverlay*>(i->second);
			baseBitamp->UpdateOpacity(opacity);
		}
	}

private:
	OverlayMap m_overlays;
	CriticalSection m_lock;
};