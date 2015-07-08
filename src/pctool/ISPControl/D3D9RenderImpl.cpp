#include "StdAfx.h"
#include "D3D9RenderImpl.h"
#include "yuv2rgb.h"

class CFullScreenWnd : public CDialog
{
public:
	CFullScreenWnd() 
	: CDialog(), m_pCallBack(NULL), m_pCallBackParam(NULL)
	{
	}

	void SetMessageCallBack(FULLSCREENMESSAGE pCallBack, void * pParam)
	{
		m_pCallBack = pCallBack;
		m_pCallBackParam = pParam;
	}

protected:
	
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (m_pCallBack)
			m_pCallBack(message, wParam, lParam, m_pCallBackParam);

		return CDialog::WindowProc(message, wParam, lParam);
	}
	
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point)
	{
		if (m_pCallBack)
			m_pCallBack(WM_LBUTTONDBLCLK, nFlags, MAKELPARAM(point.x, point.y), m_pCallBackParam);
	}

	DECLARE_MESSAGE_MAP()

public:
	FULLSCREENMESSAGE m_pCallBack;
	void * m_pCallBackParam;
};

BEGIN_MESSAGE_MAP(CFullScreenWnd, CDialog)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()


D3D9RenderImpl::D3D9RenderImpl()
    : m_pVertexShader(0), m_pPixelShader(0), m_pD3D9Ex(0), m_pD3D9(0), m_pDeviceEx(0), m_pDevice(0), m_format(D3DFMT_UNKNOWN), m_pVertexConstantTable(0), m_pPixelConstantTable(0), 
      m_fillMode(KeepAspectRatio), m_deviceIsLost(false), m_pResetCallback(NULL), m_pLostCallback(NULL), m_adapterId(0), m_showOverlays(true)
{
	m_devType = D3DDEVTYPE_HAL;
	m_videoWidth = m_videoHeight = 0;
	m_hSaveWindow = NULL;
	m_pFullScreenWnd = NULL;
	m_bIsNeedConvert = FALSE;
#ifdef USE_FFMPEG_SCALE
	m_pcScale = NULL;
	m_pScaleSrc = NULL;
	m_nScaleSrcSzie = 0;
#endif
	m_destroy = FALSE;
}

HRESULT D3D9RenderImpl::Initialize(HWND hDisplayWindow, int adapterId)
{
	m_devType = D3DDEVTYPE_HAL;
	if(hDisplayWindow != NULL && IsWindow(hDisplayWindow))
	{
		m_hDisplayWindow = hDisplayWindow;
		//m_renderModeIsWindowed = true;
	}
	else
	{
		CreateDummyWindow();
		//m_renderModeIsWindowed = true;
	}

	m_renderModeIsWindowed = true;

	m_adapterId = adapterId;
    //HR(Direct3DCreate9Ex( D3D_SDK_VERSION, &m_pD3D9Ex ));
	HRESULT hr = S_OK;
	hr = EnsureD3DObject(); //创建D3D
	if ( m_pD3D9Ex ) {
		hr = m_pD3D9Ex->GetAdapterDisplayMode(adapterId, &m_displayMode);
	} else { 
		hr = m_pD3D9->GetAdapterDisplayMode(adapterId, &m_displayMode);
	}

	if (FAILED(hr)) {
		//WCHAR afxmsg[100];
		//_sntprintf( afxmsg, 100, L"no support GetAdapterDisplayMode hr = %d", hr);
		//AfxMessageBox( afxmsg, 0, 0 );
		fprintf(stderr, "%s::%s no support GetAdapterDisplayMode hr = %d", __FILE__, __FUNCTION__, hr);
		return hr;
	}

	if (D3DFMT_X8R8G8B8 != m_displayMode.Format) m_displayMode.Format = D3DFMT_X8R8G8B8;
	
    D3DCAPS9 deviceCaps;
	while (TRUE) {//查询设备能力
		if ( m_pD3D9Ex ) {
			hr = m_pD3D9Ex->GetDeviceCaps(adapterId, m_devType, &deviceCaps);
		} else {
			hr = m_pD3D9->GetDeviceCaps(adapterId, m_devType, &deviceCaps);
		}

		if (FAILED(hr)) {
			ZeroMemory(&deviceCaps, sizeof(deviceCaps));

			if (m_devType == D3DDEVTYPE_HAL) {
				m_devType = D3DDEVTYPE_REF;
				continue;
			}else {
				//WCHAR afxmsg[100];
				//_sntprintf( afxmsg, 100, L"Deveice can't do anything!\n", hr);
				//AfxMessageBox( afxmsg, 0, 0 );
				fprintf( stderr, "%s::%s Deveice can't do anything!\n", __FILE__, __FUNCTION__, hr);
			}
		}

		break;
	}

	DWORD dwBehaviorFlags = D3DCREATE_MULTITHREADED;

	if(!m_renderModeIsWindowed)
	{
		dwBehaviorFlags |= D3DCREATE_FPU_PRESERVE;
	}

    if( deviceCaps.VertexProcessingCaps != 0 )
        dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
    else
        dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    
    hr = GetPresentParams(&m_presentParams, TRUE);
	if (FAILED(hr)) {
		//WCHAR afxmsg[100];
		//_sntprintf( afxmsg, 100, L"no support GetPresentParams hr = %d", hr);
		//AfxMessageBox( afxmsg, 0, 0 );
		fprintf(stderr, "%s::%s no support GetPresentParams hr = %d", __FILE__, __FUNCTION__, hr);
		return hr;
	}
    
	if ( m_pD3D9Ex ) {
		hr = m_pD3D9Ex->CreateDeviceEx(adapterId, m_devType, m_hDisplayWindow, dwBehaviorFlags, &m_presentParams, NULL, &m_pDeviceEx);
	} else {
		hr = m_pD3D9->CreateDevice(adapterId, m_devType, m_hDisplayWindow, dwBehaviorFlags, &m_presentParams, &m_pDevice);
		if (FAILED(hr)) { // this device is to old, just use the default function
			int width = m_presentParams.BackBufferWidth;
			int height = m_presentParams.BackBufferHeight;
			ZeroMemory(&m_presentParams, sizeof(D3DPRESENT_PARAMETERS));

			//if (m_renderModeIsWindowed)
				//m_presentParams.Windowed = TRUE;
			//else
				//m_presentParams.Windowed = FALSE;
			
			m_presentParams.Windowed = TRUE;
			m_presentParams.hDeviceWindow = m_hDisplayWindow;
			m_presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
			m_presentParams.Flags = D3DPRESENTFLAG_DEVICECLIP | D3DPRESENTFLAG_VIDEO;
			m_presentParams.BackBufferWidth	= width;
			m_presentParams.BackBufferHeight = height;
			m_presentParams.BackBufferFormat = m_displayMode.Format;
			m_presentParams.BackBufferCount = 1;
			m_presentParams.EnableAutoDepthStencil = FALSE;
			m_presentParams.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
			hr = m_pD3D9->CreateDevice(D3DADAPTER_DEFAULT, m_devType, m_hDisplayWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_presentParams, &m_pDevice);
		}
	}

	if (FAILED(hr)) {
		//WCHAR afxmsg[100];
		//_sntprintf( afxmsg, 100, L"no support CreateDevice hr = %d", hr);
		//AfxMessageBox( afxmsg, 0, 0 );
		fprintf(stderr, "%s::%s no support CreateDevice hr = %d", __FILE__, __FUNCTION__, hr);
		return hr;
	}

	if (D3DDEVTYPE_REF == m_devType) {
		//WCHAR afxmsg[100];
		//_sntprintf( afxmsg, 100, L"警告!当前PC上的显卡设备性能过低,虽然支持D3D9但速度可能极慢!\n");
		//AfxMessageBox( afxmsg, 0, 0 );
		fprintf(stderr, "%s::%s WARN! your pc is display cart too old, it can support d3d9 but very slow!\n");
	}

	if(!m_renderModeIsWindowed)
		return S_OK;
	
	return CreateResources(m_presentParams.BackBufferWidth, m_presentParams.BackBufferHeight);
}

D3D9RenderImpl::~D3D9RenderImpl(void)
{
	DiscardResources();
	if (m_pD3D9Ex)		SafeRelease(m_pD3D9Ex);
	if (m_pD3D9)		SafeRelease(m_pD3D9);
    if (m_pDeviceEx)	SafeRelease(m_pDeviceEx);
	if (m_pDevice)		SafeRelease(m_pDevice);

	if (m_pFullScreenWnd) {
		if (::IsWindow(m_pFullScreenWnd->GetSafeHwnd())) {
			if (m_pFullScreenWnd->m_pCallBack) {
				m_destroy = TRUE;
				m_pFullScreenWnd->m_pCallBack(WM_KEYUP, VK_ESCAPE, 0, m_pFullScreenWnd->m_pCallBackParam);
			}

			m_pFullScreenWnd->CloseWindow();
			m_pFullScreenWnd->SendMessage(WM_CLOSE, 0, 0);
		}
		delete m_pFullScreenWnd;
	}
	
	m_pFullScreenWnd = NULL;

#ifdef USE_FFMPEG_SCALE
	if (m_pcScale) delete m_pcScale;
	m_pcScale = NULL;

	if (m_pScaleSrc) delete[] m_pScaleSrc;
	m_pScaleSrc = NULL;
	m_nScaleSrcSzie = 0;
#endif
}

void D3D9RenderImpl::ReleaseInstance()
{
	delete this;
}

HRESULT D3D9RenderImpl::FullScreen(BOOL bFullScreen, int adapterId, FULLSCREENMESSAGE pMessageCallBack, 
								   void * pCallBackParam)
{
	if (m_destroy) return S_OK;

	CAutoLock lock(&m_locker);
	HRESULT hr = S_OK;

	if (D3DDEVTYPE_REF == m_devType) {
		WCHAR afxmsg[100];
		_sntprintf( afxmsg, 100, L"当前PC上的显卡设备性能过低,程序在这种显卡下不支持全屏!\n");
		AfxMessageBox( afxmsg, 0, 0 );
		return E_FAIL;
	}

	if (m_videoWidth == 0 || m_videoHeight == 0 || 
		(m_pDeviceEx == NULL && m_pDevice == NULL)) {
		return E_FAIL;
	}
	
	Clear(D3DCOLOR_ARGB(0xFF, 0, 0, 0));

	DiscardResources();

	if (m_pDeviceEx)	SafeRelease(m_pDeviceEx);
	if (m_pDevice)		SafeRelease(m_pDevice);

	if (bFullScreen) {
		m_hSaveWindow = m_hDisplayWindow;
		CreateDummyWindow();
		GetPresentParams(&m_presentParams, FALSE);
		m_pFullScreenWnd->SetMessageCallBack(pMessageCallBack, pCallBackParam);

		//m_renderModeIsWindowed = false;
	}else {
		if ((m_hSaveWindow == NULL) || (!IsWindow(m_hSaveWindow))) return E_FAIL;

		if ((m_hDisplayWindow!= NULL) && IsWindow(m_hDisplayWindow) && m_pFullScreenWnd) {
			m_pFullScreenWnd->DestroyWindow();
		}

		m_hDisplayWindow = m_hSaveWindow;
		GetPresentParams(&m_presentParams, TRUE);
		//m_renderModeIsWindowed = true;
	}

	m_renderModeIsWindowed = true;

	D3DCAPS9 deviceCaps;
	if ( m_pD3D9Ex ) {
		hr = m_pD3D9Ex->GetDeviceCaps(adapterId, m_devType, &deviceCaps);
	} else {
		hr = m_pD3D9->GetDeviceCaps(adapterId, m_devType, &deviceCaps);
	}

	if (FAILED(hr)) {
		WCHAR afxmsg[100];
		_sntprintf( afxmsg, 100, L"no support GetDeviceCaps hr = %d", hr);
		AfxMessageBox( afxmsg, 0, 0 );
		return -1;
	}

	DWORD dwBehaviorFlags = D3DCREATE_MULTITHREADED;

	if(!m_renderModeIsWindowed)
	{
		dwBehaviorFlags |= D3DCREATE_FPU_PRESERVE;
	}

	if( deviceCaps.VertexProcessingCaps != 0 )
		dwBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		dwBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	if ( m_pD3D9Ex ) {
		hr = m_pD3D9Ex->CreateDeviceEx(adapterId, m_devType, m_hDisplayWindow, dwBehaviorFlags, &m_presentParams, NULL, &m_pDeviceEx);
	} else {
		hr = m_pD3D9->CreateDevice(adapterId, m_devType, m_hDisplayWindow, dwBehaviorFlags, &m_presentParams, &m_pDevice);
		if (FAILED(hr)) { // this device is too old, just use the default function
			int width = m_presentParams.BackBufferWidth;
			int height = m_presentParams.BackBufferHeight;
			ZeroMemory(&m_presentParams, sizeof(D3DPRESENT_PARAMETERS));

			//if (m_renderModeIsWindowed)
				//m_presentParams.Windowed = TRUE;
			//else
				//m_presentParams.Windowed = FALSE;
			m_presentParams.Windowed = TRUE;
			m_presentParams.hDeviceWindow = m_hDisplayWindow;
			m_presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
			m_presentParams.Flags = D3DPRESENTFLAG_DEVICECLIP | D3DPRESENTFLAG_VIDEO;
			m_presentParams.BackBufferWidth	= width;
			m_presentParams.BackBufferHeight = height;
			m_presentParams.BackBufferFormat = m_displayMode.Format;
			m_presentParams.BackBufferCount = 1;
			m_presentParams.EnableAutoDepthStencil = FALSE;
			m_presentParams.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
			hr = m_pD3D9->CreateDevice(D3DADAPTER_DEFAULT, m_devType, m_hDisplayWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_presentParams, &m_pDevice);
		}
	}

	HR(CreateVideoSurface(m_videoWidth, m_videoHeight, m_format));

	if (m_renderModeIsWindowed)
		hr = CreateResources(m_presentParams.BackBufferWidth, m_presentParams.BackBufferHeight);
	
	if (FAILED(hr)) return hr;

	SetDisplayMode(m_fillMode);

	Clear(D3DCOLOR_ARGB(0xFF, 0, 0, 0));

	return S_OK;
}

HRESULT D3D9RenderImpl::CheckFormatConversion(D3DFORMAT format)
{
	if ( m_pD3D9Ex ) {
		HR(m_pD3D9Ex->CheckDeviceFormat(m_adapterId, m_devType, m_displayMode.Format, 0, D3DRTYPE_SURFACE, format));
	} else {
		HR(m_pD3D9->CheckDeviceFormat(m_adapterId, m_devType, m_displayMode.Format, 0, D3DRTYPE_SURFACE, format));
	}
	
	if ( m_pD3D9Ex )	return m_pD3D9Ex->CheckDeviceFormatConversion(m_adapterId, m_devType, format, m_displayMode.Format);
	else				return m_pD3D9->CheckDeviceFormatConversion(m_adapterId, m_devType, format, m_displayMode.Format);
}


HRESULT D3D9RenderImpl::CreateRenderTarget(int width, int height)
{
	HRESULT hr = S_OK;

	if ( m_pDeviceEx ) {
		hr = m_pDeviceEx->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, m_displayMode.Format, D3DPOOL_DEFAULT, &m_pTexture, NULL);
	}else {
		hr = m_pDevice->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, m_displayMode.Format, D3DPOOL_DEFAULT, &m_pTexture, NULL);
	}

	if (FAILED(hr)) {
		WCHAR afxmsg[100];
		_sntprintf( afxmsg, 100, L"no support CreateTexture hr = %d", hr);
		AfxMessageBox( afxmsg, 0, 0 );
		return hr;
	}

	hr = m_pTexture->GetSurfaceLevel(0, &m_pTextureSurface);
	if (FAILED(hr)) {
		WCHAR afxmsg[100];
		_sntprintf( afxmsg, 100, L"no support GetSurfaceLevel hr = %d", hr);
		AfxMessageBox( afxmsg, 0, 0 );
		return hr;
	}

	if ( m_pDeviceEx ) {
		hr = m_pDeviceEx->CreateVertexBuffer(sizeof(VERTEX) * 4, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL);
	} else {
		hr = m_pDevice->CreateVertexBuffer(sizeof(VERTEX) * 4, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL);
	}

	if (FAILED(hr)) {
		WCHAR afxmsg[100];
		_sntprintf( afxmsg, 100, L"no support CreateVertexBuffer hr = %d", hr);
		AfxMessageBox( afxmsg, 0, 0 );
		return hr;
	}

    VERTEX vertexArray[] =
    {
        { D3DXVECTOR3(0, 0, 0),          D3DCOLOR_ARGB(255, 255, 255, 255), D3DXVECTOR2(0, 0) },  // top left
        { D3DXVECTOR3(width, 0, 0),      D3DCOLOR_ARGB(255, 255, 255, 255), D3DXVECTOR2(1, 0) },  // top right
        { D3DXVECTOR3(width, height, 0), D3DCOLOR_ARGB(255, 255, 255, 255), D3DXVECTOR2(1, 1) },  // bottom right
        { D3DXVECTOR3(0, height, 0),     D3DCOLOR_ARGB(255, 255, 255, 255), D3DXVECTOR2(0, 1) },  // bottom left
    };

    VERTEX *vertices;
    hr = m_pVertexBuffer->Lock(0, 0, (void**)&vertices, D3DLOCK_DISCARD);

	if (FAILED(hr)) {
		WCHAR afxmsg[100];
		_sntprintf( afxmsg, 100, L"no support Lock hr = %d", hr);
		AfxMessageBox( afxmsg, 0, 0 );
		return hr;
	}

    memcpy(vertices, vertexArray, sizeof(vertexArray));
	
	hr = m_pVertexBuffer->Unlock();
	if (FAILED(hr)) {
		WCHAR afxmsg[100];
		_sntprintf( afxmsg, 100, L"no support Unlock hr = %d", hr);
		AfxMessageBox( afxmsg, 0, 0 );
		return hr;
	}

    return S_OK;
}

HRESULT D3D9RenderImpl::Display(BYTE* pYplane, int iYlineSize, BYTE* pVplane, int iVLineSize, BYTE* pUplane, int iULineSize)
{
	if (m_pDevice == NULL) return S_OK;
	CAutoLock lock(&m_locker);

    if(!pYplane)
    {
        return E_POINTER;
    }

    if(m_format == D3DFMT_NV12 && !pVplane)
    {
        return E_POINTER;
    }

    if(m_format == D3DFMT_YV12 && (!pVplane || !pUplane))
    {
        return E_POINTER;
    }

    HR(FillBuffer(pYplane, iYlineSize, pVplane, iVLineSize, pUplane, iULineSize));
	HR(CreateScene());
    return Present();
}

HRESULT D3D9RenderImpl::DisplayPitch(BYTE* pPlanes[4], int pitches[4])
{
	CAutoLock lock(&m_locker);

	HR(FillBuffer(pPlanes, pitches));
	HR(CreateScene());
	return Present();
}

HRESULT D3D9RenderImpl::SetupMatrices(int width, int height)
{
	HRESULT hr = S_OK;

    D3DXMATRIX matOrtho; 
    D3DXMATRIX matIdentity;

    D3DXMatrixOrthoOffCenterLH(&matOrtho, 0, width, height, 0, 0.0f, 1.0f);
    D3DXMatrixIdentity(&matIdentity);
	
	if ( m_pDeviceEx ) {
		HR(m_pDeviceEx->SetTransform(D3DTS_PROJECTION, &matOrtho));
		HR(m_pDeviceEx->SetTransform(D3DTS_WORLD, &matIdentity));
		return m_pDeviceEx->SetTransform(D3DTS_VIEW, &matIdentity);
	}else {
		hr = m_pDevice->SetTransform(D3DTS_PROJECTION, &matOrtho);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetTransform hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
			return hr;
		}

		hr = m_pDevice->SetTransform(D3DTS_WORLD, &matIdentity);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetTransform 2 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
			return hr;
		}
		
		hr = m_pDevice->SetTransform(D3DTS_VIEW, &matIdentity);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetTransform 3 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
			return hr;
		}

		return S_OK;
	}
}

HRESULT D3D9RenderImpl::CreateScene(void)
{
	if(NULL == m_pDevice && NULL == m_pDeviceEx)
	{
		return E_POINTER;
	}
	
	HRESULT hr = E_FAIL;
	
	if ( m_pDeviceEx ) {
		hr = m_pDeviceEx->Clear(m_adapterId, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

		HR(m_pDeviceEx->BeginScene());
		hr = m_pDeviceEx->SetFVF(D3DFVF_CUSTOMVERTEX);
	}else {
		hr = m_pDevice->Clear(m_adapterId, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

		HR(m_pDevice->BeginScene());
		hr = m_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);	
	}

    if(FAILED(hr))
    {
		if ( m_pDeviceEx )
			m_pDeviceEx->EndScene();
		else
			m_pDevice->EndScene();

        return hr;
    }
	
	if ( m_pDeviceEx )
		hr = m_pDeviceEx->SetVertexShader(m_pVertexShader);
	else
		hr = m_pDevice->SetVertexShader(m_pVertexShader);

    if(FAILED(hr))
    {
		if ( m_pDeviceEx )
			m_pDeviceEx->EndScene();
		else
			m_pDevice->EndScene();

        return hr;
    }
    
	if ( m_pDeviceEx )	hr = m_pDeviceEx->SetPixelShader(m_pPixelShader);
	else				hr = m_pDevice->SetPixelShader(m_pPixelShader);

    if(FAILED(hr))
    {
		if ( m_pDeviceEx )
			m_pDeviceEx->EndScene();
		else
			m_pDevice->EndScene();

        return hr;
    }
	
	if ( m_pDeviceEx )	hr = m_pDeviceEx->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(VERTEX));
	else				hr = m_pDevice->SetStreamSource(0, m_pVertexBuffer, 0, sizeof(VERTEX));

    if(FAILED(hr))
    {
       	if ( m_pDeviceEx )
			m_pDeviceEx->EndScene();
		else
			m_pDevice->EndScene();

        return hr;
    }
	
	if ( m_pDeviceEx )	hr = m_pDeviceEx->SetTexture(0, m_pTexture);
	else				hr = m_pDevice->SetTexture(0, m_pTexture);

    if(FAILED(hr))
    {
       	if ( m_pDeviceEx )
			m_pDeviceEx->EndScene();
		else
			m_pDevice->EndScene();

        return hr;
    }
	
	if ( m_pDeviceEx )	hr = m_pDeviceEx->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	else				hr = m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

    if(FAILED(hr))
    {
       	if ( m_pDeviceEx )
			m_pDeviceEx->EndScene();
		else
			m_pDevice->EndScene();

        return hr;
    }

	if(m_showOverlays)
		m_overlays.Draw();
	
	if ( m_pDeviceEx )	return m_pDeviceEx->EndScene();
	else				return m_pDevice->EndScene();
}

HRESULT D3D9RenderImpl::OnWindowSizeChanged()
{
	CAutoLock lock(&m_locker);
	
	DiscardResources();

	D3DPRESENT_PARAMETERS pp;
	HR(GetPresentParams(&pp, TRUE));
	
	if ( m_pDeviceEx ) {
		HR(m_pDeviceEx->ResetEx(&pp, NULL));
	}else {
		HR(m_pDevice->Reset(&pp));
	}

	m_presentParams = pp;

	HR(CreateVideoSurface(m_videoWidth, m_videoHeight, m_format));

	return CreateResources(pp.BackBufferWidth, pp.BackBufferHeight);
}

HRESULT D3D9RenderImpl::CheckDevice(void)
{
	if(NULL == m_pDevice && NULL == m_pDeviceEx)
	{
		return E_POINTER;
	}
	
	HRESULT hr = E_FAIL;
	if ( NULL != m_pDeviceEx )
		hr = m_pDeviceEx->CheckDeviceState(m_hDisplayWindow);
	else
		hr = m_pDevice->TestCooperativeLevel();

	if(hr == D3DERR_DEVICELOST)
	{
		if(NULL != m_pLostCallback && !m_deviceIsLost)
		{
			(*m_pLostCallback)(m_pCallBackParam);
		}
		
		m_deviceIsLost = true;
	}
	else if(hr == D3DERR_DEVICENOTRESET)
	{
		if (m_pDevice) {
			m_deviceIsLost = false;
			(*m_pResetCallback)(m_pCallBackParam);
		}

		if(NULL != m_pResetCallback && m_deviceIsLost)
		{
			(*m_pResetCallback)(m_pCallBackParam);
		}
		
		m_deviceIsLost = false;
	}

	return hr;
}

HRESULT D3D9RenderImpl::DiscardResources()
{
    SafeRelease(m_pOffsceenSurface);
    SafeRelease(m_pTextureSurface);
    SafeRelease(m_pTexture);
    SafeRelease(m_pVertexBuffer);
    SafeRelease(m_pVertexShader); 
    SafeRelease(m_pVertexConstantTable); 
    SafeRelease(m_pPixelConstantTable); 
    SafeRelease(m_pPixelShader);

    m_overlays.RemoveAll();

    return S_OK;
}

HRESULT D3D9RenderImpl::CreateResources(int width, int height)
{
	if ( m_pDeviceEx ) {
		m_pDeviceEx->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
		m_pDeviceEx->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE);
		m_pDeviceEx->SetRenderState( D3DRS_LIGHTING, FALSE);
		m_pDeviceEx->SetRenderState( D3DRS_DITHERENABLE, TRUE);
		m_pDeviceEx->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE);

		m_pDeviceEx->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
		m_pDeviceEx->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pDeviceEx->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		m_pDeviceEx->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDeviceEx->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDeviceEx->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		m_pDeviceEx->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		m_pDeviceEx->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		m_pDeviceEx->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pDeviceEx->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_SPECULAR);

		m_pDeviceEx->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		m_pDeviceEx->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		m_pDeviceEx->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	}else {
		HRESULT hr = S_OK;

		hr = m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetRenderState 1 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetRenderState 2 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetRenderState( D3DRS_LIGHTING, FALSE);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetRenderState 3 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetRenderState 3 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetRenderState 4 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetRenderState 5 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetRenderState 6 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetRenderState 7 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetSamplerState 1 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetSamplerState 2 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetSamplerState 3 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetSamplerState 4 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetTextureStageState 1 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetTextureStageState 2 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_SPECULAR);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetTextureStageState 3 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetTextureStageState 4 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetTextureStageState 5 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}

		hr = m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		if (FAILED(hr)) {
			WCHAR afxmsg[100];
			_sntprintf( afxmsg, 100, L"no support SetTextureStageState 5 hr = %d", hr);
			AfxMessageBox( afxmsg, 0, 0 );
		}
	}

    HRESULT hr = CreateRenderTarget(width, height);
	if (FAILED(hr)) {
		WCHAR afxmsg[100];
		_sntprintf( afxmsg, 100, L"no support CreateRenderTarget hr = %d", hr);
		AfxMessageBox( afxmsg, 0, 0 );
		return hr;
	}

    hr = SetupMatrices(width, height);
	if (FAILED(hr)) {
		WCHAR afxmsg[100];
		_sntprintf( afxmsg, 100, L"no support CreateRenderTarget hr = %d", hr);
		AfxMessageBox( afxmsg, 0, 0 );
		return hr;
	}

    return S_OK;
}

static inline void ApplyLetterBoxing(RECT& rendertTargetArea, FLOAT frameWidth, FLOAT frameHeight)
{
    const float aspectRatio = frameWidth / frameHeight;

    const float targetW = fabs((FLOAT)(rendertTargetArea.right - rendertTargetArea.left));
    const float targetH = fabs((FLOAT)(rendertTargetArea.bottom - rendertTargetArea.top));

    float tempH = targetW / aspectRatio;    
            
    if(tempH <= targetH)
    {               
        float deltaH = fabs(tempH - targetH) / 2;
        rendertTargetArea.top += deltaH;
        rendertTargetArea.bottom -= deltaH;
    }
    else
    {
        float tempW = targetH * aspectRatio;    
        float deltaW = fabs(tempW - targetW) / 2;

        rendertTargetArea.left += deltaW;
        rendertTargetArea.right -= deltaW;
    }
}

void D3D9RenderImpl::ConvertFormat(BYTE* pPlanes[4], int pitches[4], D3DLOCKED_RECT & stRectDest)
{
	//only support yuv420p to ARGB
	if ((m_format != D3DFMT_YV12) || (m_displayMode.Format != D3DFMT_X8R8G8B8)) return;

#ifdef USE_FFMPEG_SCALE //FFMPEG SCALE存在效率问题
	if (m_pcScale == NULL) {
		m_pcScale = new CFFScale();
		m_pcScale->SetAttribute(SWS_PF_YUV420P, SWS_PF_BGRA, SWS_SA_FAST_BILINEAR);
	}
	
	int nScaleSrcSzie = (m_videoWidth * m_videoHeight * 3) >> 1;

	if (NULL == m_pScaleSrc || nScaleSrcSzie != m_nScaleSrcSzie) {
		if (m_pScaleSrc) delete[] m_pScaleSrc;
		m_nScaleSrcSzie = nScaleSrcSzie;
		m_pScaleSrc = new BYTE[m_nScaleSrcSzie];
	}

	if (NULL == m_pScaleSrc) { 
		m_nScaleSrcSzie = 0;
		return;
	}
	
	int i = 0;
	BYTE * pPos = pPlanes[0], *pDesPos = m_pScaleSrc;
	for (; i < m_videoHeight; ++i) {
		memcpy(pDesPos, pPos, m_videoWidth);
		pPos += pitches[0];
		pDesPos += m_videoWidth; 
	}

	pPos = pPlanes[1];
	for (i = 0; i < m_videoHeight >> 1; ++i) {
		memcpy(pDesPos, pPos, m_videoWidth >> 1);
		pPos += pitches[1];
		pDesPos += (m_videoWidth >> 1);
	}
	
	pPos = pPlanes[2];
	for (i = 0; i < m_videoHeight >> 1; ++i) {
		memcpy(pDesPos, pPos, m_videoWidth >> 1);
		pPos += pitches[2];
		pDesPos += (m_videoWidth >> 1);
	}

	BOOL bScale = m_pcScale->Scale(m_pScaleSrc, m_videoWidth, m_videoHeight, 0, (BYTE *)stRectDest.pBits, m_videoWidth, m_videoHeight, stRectDest.Pitch);
#else
	/*YUV_TO_RGB32_s(pPlanes[0], pitches[0], pPlanes[1], pPlanes[2], pitches[1], 
				(BYTE *)stRectDest.pBits, m_videoWidth, m_videoHeight, stRectDest.Pitch);*/
	YUV_TO_RGB32(pPlanes[0], pitches[0], pPlanes[1], pPlanes[2], pitches[1],
				(BYTE *)stRectDest.pBits, m_videoWidth, m_videoHeight, stRectDest.Pitch);
#endif
}

HRESULT D3D9RenderImpl::FillBuffer(BYTE* pPlanes[4], int pitches[4])
{
	if(NULL == m_pOffsceenSurface)
	{
		return E_POINTER;
	}

	D3DLOCKED_RECT d3drect;
	HR(m_pOffsceenSurface->LockRect(&d3drect, NULL, 0));

	int newHeight  = m_videoHeight;
	int newWidth  = m_videoWidth;

	BYTE* pict = (BYTE*)d3drect.pBits;
	BYTE* Y = pPlanes[0];
	BYTE* V = pPlanes[1];
	BYTE* U = pPlanes[2];

	switch(m_format)
	{
	case D3DFMT_YV12:

		if (m_bIsNeedConvert) {
			ConvertFormat(pPlanes, pitches, d3drect);
			break;
		}

		for (int y = 0 ; y < newHeight ; y++)
		{
			memcpy(pict, Y, newWidth);
			pict += d3drect.Pitch;
			Y += pitches[0];
		}
		for (int y = 0 ; y < newHeight >> 1 ; y++)
		{
			memcpy(pict, V, newWidth >> 1);
			pict += d3drect.Pitch >> 1;
			V += pitches[1];
		}
		for (int y = 0 ; y < newHeight >> 1; y++)
		{
			memcpy(pict, U, newWidth >> 1);
			pict += d3drect.Pitch >> 1;
			U += pitches[2];
		}	
		break;

	case D3DFMT_NV12:

		if (m_bIsNeedConvert) break; // no support

		for (int y = 0 ; y < newHeight ; y++)
		{
			memcpy(pict, Y, newWidth);
			pict += d3drect.Pitch;
			Y += pitches[0];
		}
		for (int y = 0 ; y < newHeight >> 1 ; y++)
		{
			memcpy(pict, V, newWidth);
			pict += d3drect.Pitch;
			V += pitches[1];
		}
		break;

	case D3DFMT_YUY2:
	case D3DFMT_UYVY:
	case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
		if (m_bIsNeedConvert) break; // no support

		memcpy(pict, Y, d3drect.Pitch * newHeight);

		break;
	}

	return m_pOffsceenSurface->UnlockRect();
}

HRESULT D3D9RenderImpl::FillBuffer(BYTE* pY, int iYlineSize, BYTE* pV, int iVLineSize, BYTE* pU, int iULineSize)
{
	if(NULL == m_pOffsceenSurface)
	{
		return E_POINTER;
	}

    D3DLOCKED_RECT d3drect;
    HR(m_pOffsceenSurface->LockRect(&d3drect, NULL, 0));

    int newHeight  = m_videoHeight;
    int newWidth  = m_videoWidth;

    BYTE* pict = (BYTE*)d3drect.pBits;
    BYTE* Y = pY;
    BYTE* V = pV;
    BYTE* U = pU;

    switch(m_format)
    {
        case D3DFMT_YV12:
			
			if (m_bIsNeedConvert) {
				BYTE* pPlanes[4] = {pY, pU, pV, NULL};
				int pitches[4] = {iYlineSize, iULineSize, iVLineSize, 0};
				ConvertFormat(pPlanes, pitches, d3drect);
				break;
			}

            for (int y = 0 ; y < newHeight ; y++)
            {
                memcpy(pict, Y, iYlineSize);
                pict += d3drect.Pitch;
                Y += iYlineSize;
            }
            for (int y = 0 ; y < newHeight >> 1 ; y++)
            {
                memcpy(pict, V, iVLineSize );
                pict += d3drect.Pitch >> 1;
                V += iVLineSize;
            }
            for (int y = 0 ; y < newHeight >> 1; y++)
            {
                memcpy(pict, U, iULineSize);
                pict += d3drect.Pitch >> 1;
                U += iULineSize;
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
     
    return m_pOffsceenSurface->UnlockRect();
}


HRESULT D3D9RenderImpl::Present(void)
{
	HRESULT hr = S_OK;

	if(NULL == m_pDevice && NULL == m_pDeviceEx)
	{
		return E_POINTER;
	}
	
	if ( m_pDeviceEx ) {
		HR(m_pDeviceEx->ColorFill(m_pTextureSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0)));
	}else {
		HR(m_pDevice->ColorFill(m_pTextureSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0)));
	}

	if(m_renderModeIsWindowed)
	{
		HRESULT hr = E_FAIL;
		if ( m_pDeviceEx ) {
			hr = m_pDeviceEx->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, &m_targetRect, D3DTEXF_LINEAR);
			if (FAILED(hr)) {
				HR(m_pDeviceEx->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, &m_targetRect, D3DTEXF_NONE));	
			}

			hr = m_pDeviceEx->PresentEx(NULL, NULL, NULL, NULL, NULL);
		} else {
			
			hr = m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, &m_targetRect, D3DTEXF_LINEAR);
			if (FAILED(hr)) {
				HR(m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, &m_targetRect, D3DTEXF_NONE));	
			}

			hr = m_pDevice->Present(NULL, NULL, NULL, NULL);
		}

		if(FAILED(hr))
		{
			return CheckDevice();
		}
	}
	else
	{
		if ( m_pDeviceEx ) {
			hr = m_pDeviceEx->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, NULL, D3DTEXF_LINEAR);
			if (FAILED(hr)) {
				HR(m_pDeviceEx->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, NULL, D3DTEXF_NONE));	
			}

			hr = m_pDeviceEx->PresentEx(NULL, NULL, NULL, NULL, NULL);
		}else {
			hr = m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, NULL, D3DTEXF_LINEAR);
			if (FAILED(hr)) {
				HR(m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, NULL, D3DTEXF_NONE));	
			}

			hr = m_pDevice->Present(NULL, NULL, NULL, NULL);
		}
	} 

	return S_OK;
}


HRESULT D3D9RenderImpl::CreateVideoSurface(int width, int height, D3DFORMAT format)
{
    m_videoWidth = width;
    m_videoHeight = height;
    m_format = format;
	format =  m_displayMode.Format; //首先使用设备默认给出Format进行Surface的创建

	HRESULT hr = S_OK;
	while(TRUE) {
		if ( m_pDeviceEx ) {
			hr = m_pDeviceEx->CreateOffscreenPlainSurfaceEx(width, height, format, D3DPOOL_DEFAULT, &m_pOffsceenSurface, NULL, NULL);
		} else {
			hr = m_pDevice->CreateOffscreenPlainSurface(width, height, format, D3DPOOL_DEFAULT, &m_pOffsceenSurface, NULL);
		}

		if (FAILED(hr)) {
			if (format == m_format) return hr;
			//if (format == m_displayMode.Format) return hr;
			format = m_format; //使用用户给出的format创建Surface
			//format = m_displayMode.Format;
			//m_bIsNeedConvert = TRUE;
			continue;
		}
		
		break;
	}

	if (format != m_format) m_bIsNeedConvert = TRUE;

	if (m_pDeviceEx) {
		HR(m_pDeviceEx->ColorFill(m_pOffsceenSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0)));
	}else {
		HR(m_pDevice->ColorFill(m_pOffsceenSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0)));
	}

	::GetClientRect(m_hDisplayWindow, &m_targetRect);
	ApplyLetterBoxing(m_targetRect, (FLOAT)m_videoWidth, (FLOAT)m_videoHeight);
	if(m_renderModeIsWindowed)
	{
		return S_OK;
	}
	
	return CreateResources(width, height);
}


HRESULT D3D9RenderImpl::GetPresentParams(D3DPRESENT_PARAMETERS* params, BOOL bWindowed)
{
    UINT height, width;

	bWindowed = TRUE;

    if(bWindowed) // windowed mode
    {
        RECT rect;
        ::GetClientRect(m_hDisplayWindow, &rect);
        height = rect.bottom - rect.top;
        width = rect.right - rect.left;
    }
    else   // fullscreen mode
    {
        width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    }

    D3DPRESENT_PARAMETERS presentParams = {0};
	presentParams.Flags                  = D3DPRESENTFLAG_DEVICECLIP | D3DPRESENTFLAG_VIDEO /*| D3DPRESENTFLAG_OVERLAY_YCbCr_BT709*/;
    presentParams.Windowed               = bWindowed;
    presentParams.hDeviceWindow          = m_hDisplayWindow;
    presentParams.BackBufferWidth        = width == 0 ? 1 : width;
    presentParams.BackBufferHeight       = height == 0 ? 1 : height;
    presentParams.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    presentParams.MultiSampleType        = D3DMULTISAMPLE_NONMASKABLE;
	presentParams.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;
    presentParams.BackBufferFormat       = m_displayMode.Format;
    presentParams.BackBufferCount        = 1;
    presentParams.EnableAutoDepthStencil = FALSE;
	
    memcpy(params, &presentParams, sizeof(D3DPRESENT_PARAMETERS));

    return S_OK;
}

HRESULT D3D9RenderImpl::SetVertexShader(LPCWSTR pVertexShaderName, LPCSTR entryPoint, LPCSTR shaderModel, LPSTR* ppError)
{
    CComPtr<ID3DXBuffer> code;
    CComPtr<ID3DXBuffer> errMsg;

    HRESULT hr = D3DXCompileShaderFromFile(pVertexShaderName, NULL, NULL, entryPoint, shaderModel, 0, &code, &errMsg, &m_pVertexConstantTable);
    if (FAILED(hr))
    {	
        if(errMsg != NULL)
        {
            size_t len = errMsg->GetBufferSize() + 1;
            *ppError = new CHAR[len];		
            memcpy(*ppError, errMsg->GetBufferPointer(), len);			
        }
        return hr;
    }
	
	if ( m_pDeviceEx )	return m_pDeviceEx->CreateVertexShader((DWORD*)code->GetBufferPointer(), &m_pVertexShader);
	else				return m_pDevice->CreateVertexShader((DWORD*)code->GetBufferPointer(), &m_pVertexShader);
}

HRESULT D3D9RenderImpl::ApplyWorldViewProj(LPCSTR matrixName)
{
    D3DXMATRIX matOrtho;
	if ( m_pDeviceEx ) {
		HR(m_pDeviceEx->GetTransform(D3DTS_PROJECTION, &matOrtho));
	} else {
		HR(m_pDevice->GetTransform(D3DTS_PROJECTION, &matOrtho));
	}
	
	if ( m_pDeviceEx )	return m_pVertexConstantTable->SetMatrix(m_pDeviceEx, matrixName, &matOrtho);
	else				return m_pVertexConstantTable->SetMatrix(m_pDevice, matrixName, &matOrtho);
}

HRESULT D3D9RenderImpl::SetVertexShader(DWORD* buffer)
{
    HR(D3DXGetShaderConstantTable(buffer, &m_pVertexConstantTable));
	
	if ( m_pDeviceEx )	return m_pDeviceEx->CreateVertexShader(buffer, &m_pVertexShader);
	else				return m_pDevice->CreateVertexShader(buffer, &m_pVertexShader);
}

HRESULT D3D9RenderImpl::SetVertexShaderConstant(LPCSTR name, LPVOID value, UINT size)
{
	if ( m_pDeviceEx )	return m_pVertexConstantTable->SetValue(m_pDeviceEx, name, value, size);
    else				return m_pVertexConstantTable->SetValue(m_pDevice, name, value, size);
}

HRESULT D3D9RenderImpl::SetPixelShader(LPCWSTR pPixelShaderName, LPCSTR entryPoint, LPCSTR shaderModel, LPSTR* ppError)
{
    CComPtr<ID3DXBuffer> code;
    CComPtr<ID3DXBuffer> errMsg;

    HRESULT hr = D3DXCompileShaderFromFile(pPixelShaderName, NULL, NULL, entryPoint, shaderModel, 0, &code, &errMsg, &m_pPixelConstantTable);
    if (FAILED(hr))
    {	
        if(errMsg != NULL)
        {
            size_t len = errMsg->GetBufferSize() + 1;
            *ppError = new CHAR[len];		
            memcpy(*ppError, errMsg->GetBufferPointer(), len);	
        }
        return hr;
    }
	
	if ( m_pDeviceEx )	return m_pDeviceEx->CreatePixelShader((DWORD*)code->GetBufferPointer(), &m_pPixelShader);
	else				return m_pDevice->CreatePixelShader((DWORD*)code->GetBufferPointer(), &m_pPixelShader);
}

HRESULT D3D9RenderImpl::SetPixelShader(DWORD* buffer)
{
    HR(D3DXGetShaderConstantTable(buffer, &m_pPixelConstantTable));
	
	if ( m_pDeviceEx )	return m_pDeviceEx->CreatePixelShader(buffer, &m_pPixelShader);
	else				return m_pDevice->CreatePixelShader(buffer, &m_pPixelShader);
}

HRESULT D3D9RenderImpl::SetPixelShaderConstant(LPCSTR name, LPVOID value, UINT size)
{
	if ( m_pDeviceEx )	return m_pPixelConstantTable->SetValue(m_pDeviceEx, name, value, size);
	else				return m_pPixelConstantTable->SetValue(m_pDevice, name, value, size);
}

HRESULT D3D9RenderImpl::SetVertexShaderMatrix(D3DXMATRIX* matrix, LPCSTR name)
{
	if ( m_pDeviceEx )	return m_pVertexConstantTable->SetMatrix(m_pDeviceEx, name, matrix);
	else				return m_pVertexConstantTable->SetMatrix(m_pDevice, name, matrix);
}

HRESULT D3D9RenderImpl::SetPixelShaderMatrix(D3DXMATRIX* matrix, LPCSTR name)
{
	if ( m_pDeviceEx )	return  m_pPixelConstantTable->SetMatrix(m_pDeviceEx, name, matrix);
	else				return  m_pPixelConstantTable->SetMatrix(m_pDevice, name, matrix);
}
    
HRESULT D3D9RenderImpl::SetVertexShaderVector(D3DXVECTOR4* vector, LPCSTR name)
{
    if ( m_pDeviceEx )	return  m_pVertexConstantTable->SetVector(m_pDeviceEx, name, vector);
	else				return  m_pVertexConstantTable->SetVector(m_pDevice, name, vector);
}
    
HRESULT D3D9RenderImpl::SetPixelShaderVector(D3DXVECTOR4* vector, LPCSTR name)
{
	if ( m_pDeviceEx )	return  m_pPixelConstantTable->SetVector(m_pDeviceEx, name, vector);
	else				return  m_pPixelConstantTable->SetVector(m_pDevice, name, vector);
}

HRESULT D3D9RenderImpl::DrawLine(SHORT key, POINT p1, POINT p2, FLOAT width, D3DCOLOR color, BYTE opacity)
{
	if ( m_pDeviceEx )	m_overlays.AddOverlay(new LineOverlay(m_pDeviceEx, p1, p2, width, color, opacity), key);
	else				m_overlays.AddOverlay(new LineOverlay(m_pDevice, p1, p2, width, color, opacity), key);

    return S_OK;
}

HRESULT D3D9RenderImpl::DrawRectangle(SHORT key, RECT rectangle, FLOAT width, D3DCOLOR color, BYTE opacity)
{
	if ( m_pDeviceEx )	m_overlays.AddOverlay(new RectangleOverlay(m_pDeviceEx, rectangle, width, color, opacity), key);
	else				m_overlays.AddOverlay(new RectangleOverlay(m_pDevice, rectangle, width, color, opacity), key);

    return S_OK;
}

HRESULT D3D9RenderImpl::DrawPolygon(SHORT key, POINT* points, INT pointsLen, FLOAT width, D3DCOLOR color, BYTE opacity)
{
    if ( m_pDeviceEx )	m_overlays.AddOverlay(new PolygonOverlay(m_pDeviceEx, points, pointsLen, width, color, opacity), key);
	else				m_overlays.AddOverlay(new PolygonOverlay(m_pDevice, points, pointsLen, width, color, opacity), key);
    
	return S_OK;
}

HRESULT D3D9RenderImpl::DrawText(SHORT key, LPCWSTR text, RECT pos, INT size, D3DCOLOR color, LPCWSTR font, BYTE opacity)
{
    if ( m_pDeviceEx )	m_overlays.AddOverlay(new TextOverlay(m_pDeviceEx, text, pos, size, color, font, opacity), key);
	else				m_overlays.AddOverlay(new TextOverlay(m_pDevice, text, pos, size, color, font, opacity), key);

    return S_OK;
}

HRESULT D3D9RenderImpl::DrawBitmap(SHORT key, RECT rectangle, LPCWSTR path, D3DCOLOR colorKey, BYTE opacity)
{
    if ( m_pDeviceEx )	m_overlays.AddOverlay(new FileOverlay(m_pDeviceEx, rectangle, path, colorKey, opacity), key);
	else				m_overlays.AddOverlay(new FileOverlay(m_pDevice, rectangle, path, colorKey, opacity), key);

    return S_OK;
}

HRESULT D3D9RenderImpl::DrawBitmap(SHORT key, RECT rectangle, BYTE* pixelData, UINT stride, INT width, INT height, D3DCOLOR colorKey, BYTE opacity)
{
	if ( m_pDeviceEx )	m_overlays.AddOverlay(new MemoryBitmapOverlay(m_pDeviceEx, rectangle, pixelData, stride, width, height, colorKey, opacity), key);
	else				m_overlays.AddOverlay(new MemoryBitmapOverlay(m_pDevice, rectangle, pixelData, stride, width, height, colorKey, opacity), key);

	return S_OK;
}
	
	
HRESULT D3D9RenderImpl::RemoveOverlay(SHORT key)
{
    m_overlays.RemoveOverlay(key);

    return S_OK;
}

HRESULT D3D9RenderImpl::RemoveAllOverlays()
{
    m_overlays.RemoveAll();

    return S_OK;
}

HRESULT D3D9RenderImpl::CaptureDisplayFrame(BYTE* pBuffer, INT* width, INT* height, INT* stride)
{
    CComPtr<IDirect3DSurface9> pTargetSurface;	
    CComPtr<IDirect3DSurface9> pTempSurface;
	
	if ( m_pDeviceEx ) {
		HR(m_pDeviceEx->GetRenderTarget(0, &pTargetSurface));
	}else {
		HR(m_pDevice->GetRenderTarget(0, &pTargetSurface));
	}

    D3DSURFACE_DESC desc;		
    HR(pTargetSurface->GetDesc(&desc));	

    if(!pBuffer)
    {
        *width = desc.Width;
        *height = desc.Height;
        *stride = desc.Width * 4; // Always ARGB32
        return S_OK;
    }
	
	if ( m_pDeviceEx ) {
		HR(m_pDeviceEx->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pTempSurface, NULL));				
		HR(m_pDeviceEx->GetRenderTargetData(pTargetSurface, pTempSurface));					
	} else {
		HR(m_pDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &pTempSurface, NULL));				
		HR(m_pDevice->GetRenderTargetData(pTargetSurface, pTempSurface));	
	}

    D3DLOCKED_RECT d3drect;
    HR(pTempSurface->LockRect(&d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));		
    
    BYTE* pFrame = (BYTE*)d3drect.pBits;
    BYTE* pBuf = pBuffer;
    
    memcpy(pBuf, pFrame, desc.Height * d3drect.Pitch);

    return pTempSurface->UnlockRect();
}

HRESULT D3D9RenderImpl::CaptureVideoFrame(BYTE* pBuffer, INT* width, INT* height, INT* stride)
{
    if(!pBuffer)
    {
        *width = m_videoWidth;
        *height = m_videoHeight;
        *stride = m_videoWidth * 4; // Always ARGB32
        return S_OK;
    }

    CComPtr<IDirect3DSurface9> pTempSurface;
	if ( m_pDeviceEx ) {
		HR(m_pDeviceEx->CreateOffscreenPlainSurface(m_videoWidth, m_videoHeight, m_displayMode.Format, D3DPOOL_DEFAULT, &pTempSurface, NULL));
		HR(m_pDeviceEx->StretchRect(m_pOffsceenSurface, NULL, pTempSurface, NULL, D3DTEXF_LINEAR));
	} else {
		HR(m_pDevice->CreateOffscreenPlainSurface(m_videoWidth, m_videoHeight, m_displayMode.Format, D3DPOOL_DEFAULT, &pTempSurface, NULL));
		HR(m_pDevice->StretchRect(m_pOffsceenSurface, NULL, pTempSurface, NULL, D3DTEXF_LINEAR));
	}

    D3DLOCKED_RECT d3drect;
    HR(pTempSurface->LockRect(&d3drect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY));		
    
    BYTE* pFrame = (BYTE*)d3drect.pBits;
    BYTE* pBuf = pBuffer;

    memcpy(pBuf, pFrame, m_videoHeight * d3drect.Pitch);
    
    return pTempSurface->UnlockRect();
}

HRESULT D3D9RenderImpl::ClearPixelShader()
{
    SafeRelease(m_pPixelConstantTable); 
    SafeRelease(m_pPixelShader);

    return S_OK;
}

HRESULT D3D9RenderImpl::ClearVertexShader()
{
    SafeRelease(m_pVertexConstantTable); 
    SafeRelease(m_pVertexShader); 
    
    return S_OK;
}

void D3D9RenderImpl::SetDisplayMode(FillModeMe mode)
{
	if(mode == Fill)
	{
		::GetClientRect(m_hDisplayWindow, &m_targetRect);
	}
	else
	{
		::GetClientRect(m_hDisplayWindow, &m_targetRect);
		ApplyLetterBoxing(m_targetRect, (FLOAT)m_videoWidth, (FLOAT)m_videoHeight);
	}
    m_fillMode = mode;
}

FillModeMe D3D9RenderImpl::GetDisplayMode()
{
    return m_fillMode;
}

HRESULT D3D9RenderImpl::GetRenderTargetSurface(IDirect3DSurface9** ppSurface)
{
	if(m_pTextureSurface == NULL)
	{
		*ppSurface = NULL;
		return E_POINTER;
	}

	*ppSurface = m_pTextureSurface;
	return S_OK;
}

void D3D9RenderImpl::CreateDummyWindow()
{
	CWinApp * app = AfxGetApp();
	
	WNDCLASS wndclass;
	ZeroMemory(&wndclass, sizeof(wndclass));
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	//wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.lpfnWndProc = DefWindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = app->m_hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = L"MyWindowClass";

	AfxRegisterClass(&wndclass);

	if (m_pFullScreenWnd) {
		m_pFullScreenWnd->DestroyWindow();
		delete m_pFullScreenWnd;
		m_pFullScreenWnd = NULL;
	}

	m_pFullScreenWnd = new CFullScreenWnd();

	m_pFullScreenWnd->CreateEx( WS_EX_TOPMOST, L"MyWindowClass", L"Our D3D Program", 
								WS_POPUP, 0, 0,
								GetSystemMetrics(SM_CXVIRTUALSCREEN), 
								GetSystemMetrics(SM_CYVIRTUALSCREEN), 
								NULL, NULL);
	m_hDisplayWindow = m_pFullScreenWnd->m_hWnd;
	/*
	m_hDisplayWindow = CreateWindowEx(NULL, L"WindowClass",
		L"Our D3D Program",
		WS_OVERLAPPEDWINDOW,
		//WS_EX_TOPMOST | WS_POPUP,
		0,                   // X
		0,                   // Y
		0,                   // Width
		0,                   // Height
		NULL,
		NULL,
		app->m_hInstance,
		NULL);*/

	//DWORD dwError = ::GetLastError();
	ShowWindow(m_hDisplayWindow, SW_SHOW);
	m_pFullScreenWnd->SetFocus();
}

HRESULT D3D9RenderImpl::AddOverlayStream(SHORT key, int videoWidth, int videoHeight, D3DFORMAT videoFormat, RECT targetRect, BYTE opacity)
{
	Overlay * overlay = NULL;

	if ( m_pDeviceEx )	overlay = new VideoOverlay(m_pDeviceEx, videoWidth, videoHeight, videoFormat, targetRect, opacity);
	else				overlay = new VideoOverlay(m_pDevice, videoWidth, videoHeight, videoFormat, targetRect, opacity);

	m_overlays.AddOverlay(overlay, key);

	return S_OK;
}

HRESULT D3D9RenderImpl::UpdateOverlayFrame(SHORT key, BYTE* pYplane, BYTE* pVplane, BYTE* pUplane)
{
	m_overlays.UpdateOverlay(key, pYplane, pVplane, pUplane);
	return S_OK;
}

void D3D9RenderImpl::UpdateOverlayOpacity(SHORT key, BYTE opacity)
{
	m_overlays.UpdateOverlayOpacity(key, opacity);
}

HRESULT D3D9RenderImpl::Clear(D3DCOLOR color)
{
	if ( m_pDeviceEx )	m_pDeviceEx->ColorFill(m_pTextureSurface, NULL, color);
	else				m_pDevice->ColorFill(m_pTextureSurface, NULL, color);

	HR(CreateScene());
	return Present();
}

void D3D9RenderImpl::SetShowOverlays(bool bShow)
{
	m_showOverlays = bShow;
}

bool D3D9RenderImpl::GetShowOverlays()
{
	return m_showOverlays;
}

HRESULT D3D9RenderImpl::CreateVideoSurfaces(int width, int height, D3DFORMAT format, IDirect3DTexture9** pSurfaces, int surfaceCount)
{
	m_videoWidth = width;
	m_videoHeight = height;
	m_format = format;

	pSurfaces = new IDirect3DTexture9*[surfaceCount];
	for (int i = 0; i < surfaceCount; i++)
	{
		if ( m_pDeviceEx ) {
			HR(m_pDeviceEx->CreateTexture(width, height, 1, 0, format, D3DPOOL_DEFAULT, &pSurfaces[i], NULL));
		}else {
			HR(m_pDevice->CreateTexture(width, height, 1, 0, format, D3DPOOL_DEFAULT, &pSurfaces[i], NULL));
		}
	}
	
	::GetClientRect(m_hDisplayWindow, &m_targetRect);
	ApplyLetterBoxing(m_targetRect, (FLOAT)m_videoWidth, (FLOAT)m_videoHeight);
	if(m_renderModeIsWindowed)
	{
		return S_OK;
	}
	
	return CreateResources(width, height);
}

HRESULT D3D9RenderImpl::DisplaySurface(IDirect3DTexture9* pSurface)
{
	HRESULT hr = S_OK;

	if(NULL == pSurface)
	{
		return E_POINTER;
	}

	HR(CreateScene());

	if ( m_pDeviceEx ) {
		HR(m_pDeviceEx->ColorFill(m_pTextureSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0)));
	}else {
		HR(m_pDevice->ColorFill(m_pTextureSurface, NULL, D3DCOLOR_ARGB(0xFF, 0, 0, 0)));
	}

	IDirect3DSurface9* pSurfaceLevel0 = NULL;
	HR(pSurface->GetSurfaceLevel(0, &pSurfaceLevel0));
	if(m_renderModeIsWindowed)
	{
		HRESULT hr = E_FAIL;
		if ( m_pDeviceEx ) {
			hr = m_pDeviceEx->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, &m_targetRect, D3DTEXF_LINEAR);
			if (FAILED(hr)) {
				HR(m_pDeviceEx->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, &m_targetRect, D3DTEXF_NONE));	
			}

			hr = m_pDeviceEx->PresentEx(NULL, NULL, NULL, NULL, NULL);
		}else {
			hr = m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, &m_targetRect, D3DTEXF_LINEAR);
			if (FAILED(hr)) {
				HR(m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, &m_targetRect, D3DTEXF_NONE));	
			}

			hr = m_pDevice->Present(NULL, NULL, NULL, NULL);	
		}

		if(FAILED(hr))
		{
			return CheckDevice();
		}
	}
	else
	{
		if ( m_pDeviceEx ) {
			hr = m_pDeviceEx->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, NULL, D3DTEXF_LINEAR);
			if (FAILED(hr)) {
				HR(m_pDeviceEx->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, NULL, D3DTEXF_NONE));	
			}

			hr = m_pDeviceEx->PresentEx(NULL, NULL, NULL, NULL, NULL);
		}else {
			hr = m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, NULL, D3DTEXF_LINEAR);
			if (FAILED(hr)) {
				HR(m_pDevice->StretchRect(m_pOffsceenSurface, NULL, m_pTextureSurface, NULL, D3DTEXF_NONE));	
			}

			hr = m_pDevice->Present(NULL, NULL, NULL, NULL);
		}
	} 

	return S_OK;
}

typedef HRESULT (*DIRECT3DCREATE9EXFUNCTION)( UINT, IDirect3D9Ex ** );

HRESULT D3D9RenderImpl::EnsureD3DObject()
{
	m_pD3D9 = Direct3DCreate9( D3D_SDK_VERSION );
	if ( !m_pD3D9 ) {
		WCHAR afxmsg[100];
		_sntprintf( afxmsg, 100, L"no support Direct3DCreate9");
		AfxMessageBox( afxmsg, 0, 0 );
		return E_FAIL;
	}

	return S_OK;
}