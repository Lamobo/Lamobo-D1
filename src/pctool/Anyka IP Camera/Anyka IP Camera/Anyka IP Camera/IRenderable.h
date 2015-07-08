#pragma once

#include "d3d9.h"
#include "d3dx9.h"
#include "Vertex.h"
#include "atlbase.h"
#include "Macros.h"
#include <windows.h>
#include <vector>
#include <iostream>
#include "MMsystem.h"

#ifdef __cplusplus

using namespace std;

enum FillModeMe
{
    KeepAspectRatio = 0,
    Fill = 1
};

struct DeviceDetails
{
	WCHAR DeviceID[256];                
	WCHAR DisplayName[256]; 
	int Index;
};

typedef void (__stdcall *DEVICECALLBACK)(void * pParam);

typedef void (__stdcall *FULLSCREENMESSAGE)(UINT message, WPARAM wParam, LPARAM lParam, void * pClassParam);

struct IDisplable
{
	virtual HRESULT Initialize(HWND hDisplayWindow, int adapterId) PURE;
	//FullScreen must call by a UI thread, because it will create a full screen window. if it do not call by a
	//UI thread the full screen window will unable to handle any message, and it will block and can't restorable.
	virtual HRESULT FullScreen(BOOL bFullScreen, int adapterId, FULLSCREENMESSAGE pMessageCallBack, void * pCallBackParam) PURE;
	virtual HRESULT CheckFormatConversion(D3DFORMAT format) PURE;
	virtual HRESULT CreateVideoSurface(int width, int height, D3DFORMAT format = D3DFMT_YV12) PURE;
	virtual HRESULT CreateVideoSurfaces(int width, int height, D3DFORMAT format, IDirect3DTexture9** pSurfaces, int surfaceCount) PURE;
	virtual HRESULT Display(BYTE* pYplane, int iYlineSize, BYTE* pVplane, int iVLineSize, BYTE* pUplane, int iULineSize) PURE;
	virtual HRESULT DisplayPitch(BYTE* pPlanes[4], int pitches[4]) PURE;
	virtual HRESULT DisplaySurface(IDirect3DTexture9* pSurface) PURE;
	virtual HRESULT CaptureDisplayFrame(BYTE* pBuffer, INT* width, INT* height, INT* stride) PURE;
	virtual HRESULT CaptureVideoFrame(BYTE* pBuffer, INT* width, INT* height, INT* stride) PURE;
	virtual void SetDisplayMode(FillModeMe mode) PURE;
    virtual FillModeMe GetDisplayMode() PURE;
	virtual HRESULT GetRenderTargetSurface(IDirect3DSurface9** ppSurface) PURE;
	virtual void RegisterDeviceLostCallback(DEVICECALLBACK pLostCallback, DEVICECALLBACK pResetCallback, void * pParam) PURE;
	virtual HRESULT Clear(D3DCOLOR color) PURE;
	virtual void ReleaseInstance() PURE;
};

struct IShaderable
{
	virtual HRESULT SetPixelShader(LPCWSTR pPixelShaderName, LPCSTR entryPoint, LPCSTR shaderModel, LPSTR* ppError) PURE;
	virtual HRESULT SetPixelShader(DWORD* buffer) PURE;
	virtual HRESULT SetPixelShaderConstant(LPCSTR name, LPVOID value, UINT size) PURE;

	virtual HRESULT SetVertexShader(LPCWSTR pVertexShaderName, LPCSTR entryPoint, LPCSTR shaderModel, LPSTR* ppError) PURE;
	virtual HRESULT SetVertexShader(DWORD* buffer) PURE;
	virtual HRESULT SetVertexShaderConstant(LPCSTR name, LPVOID value, UINT size) PURE;
	virtual HRESULT ApplyWorldViewProj(LPCSTR matrixName) PURE;

	virtual HRESULT SetVertexShaderMatrix(D3DXMATRIX* matrix, LPCSTR name) PURE;
	virtual HRESULT SetPixelShaderMatrix(D3DXMATRIX* matrix, LPCSTR name) PURE;
	virtual HRESULT SetVertexShaderVector(D3DXVECTOR4* vector, LPCSTR name) PURE;
	virtual HRESULT SetPixelShaderVector(D3DXVECTOR4* vector, LPCSTR name) PURE;

	virtual HRESULT ClearPixelShader() PURE;
	virtual HRESULT ClearVertexShader() PURE;
};

struct IOverlable
{
	virtual HRESULT DrawLine(SHORT key, POINT p1, POINT p2, FLOAT width, D3DCOLOR color, BYTE opacity) PURE;
	virtual HRESULT DrawRectangle(SHORT key, RECT rectangle, FLOAT width, D3DCOLOR color, BYTE opacity) PURE;
	virtual HRESULT DrawPolygon(SHORT key, POINT* points, INT pointsLen, FLOAT width, D3DCOLOR color, BYTE opacity ) PURE;
	virtual HRESULT DrawText(SHORT key, LPCWSTR text, RECT pos, INT size, D3DCOLOR color, LPCWSTR font, BYTE opacity) PURE;
	virtual HRESULT DrawBitmap(SHORT key, RECT rectangle, LPCWSTR path, D3DCOLOR colorKey, BYTE opacity) PURE;
	virtual HRESULT DrawBitmap(SHORT key, RECT rectangle, BYTE* pixelData, UINT stride, INT width, INT height, D3DCOLOR colorKey, BYTE opacity) PURE;	
	virtual HRESULT RemoveOverlay(SHORT key) PURE;
	virtual HRESULT RemoveAllOverlays() PURE;
	virtual void SetShowOverlays(bool bShow) PURE;
	virtual bool GetShowOverlays() PURE;
};

struct IRenderable : public IDisplable, public IShaderable, public IOverlable
{
};

struct ISoundPlayer
{
	virtual HRESULT Initialize(const WAVEFORMATEX* format, int index = 0, bool isMusicPlayback = false) PURE;
	virtual HRESULT QueueSamples(BYTE* buffer, UINT32 size) PURE;
	virtual HRESULT PlayQueuedSamples() PURE;
	virtual HRESULT PlaySamples(BYTE* buffer, UINT32 size) PURE;
	virtual float GetVolume() PURE;
	virtual HRESULT SetVolume(float volume) PURE;
	virtual void GetChannelVolumes(int channels, float* volumes) PURE;
	virtual HRESULT SetChannelVolumes(int channels, const float* volumes) PURE;
	virtual vector<DeviceDetails*>& GetAudioDevices() PURE;
	virtual void ReleaseInstance() PURE;
	virtual HRESULT FlushBuffers() PURE;
	virtual HRESULT GetCurrentQueuedCnt(unsigned int & nCnt) PURE;
};

class CAudioStreamHandler {
public:
	 virtual void AdoStreamData(unsigned char * pBuffer, DWORD & nBufferLen) = 0;
};

#endif // __cplusplus