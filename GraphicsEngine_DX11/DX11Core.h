#pragma once
#include "ICore.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

// Effect
#ifdef _WIN64
	#ifdef _DEBUG
		#pragma comment(lib, "Effects11_x64_debug.lib")
	#else
		#pragma comment(lib, "Effects11_x64_release.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "Effects11_x86_debug.lib")
	#else
		#pragma comment(lib, "Effects11_x86_release.lib")
	#endif
#endif

// DXTK
#ifdef _WIN64
	#ifdef _DEBUG
		#pragma comment(lib, "DirectXTK_x64_debug.lib")
	#else
		#pragma comment(lib, "DirectXTK_x64_release.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "DirectXTK_x86_debug.lib")
	#else
		#pragma comment(lib, "DirectXTK_x86_release.lib")
	#endif
#endif

// Utility
#ifdef _WIN64
	#ifdef _DEBUG
		#pragma comment(lib, "Utility_x64_debug.lib")
	#else
		#pragma comment(lib, "Utility_x64_release.lib")
#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "Utility_x86_debug.lib")
	#else
		#pragma comment(lib, "Utility_x86_release.lib")
	#endif
#endif

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>

#include <d3dcompiler.h>

#include <vector>
#include <wrl.h>
#include <memory>

/// <summary>
/// Device를 생성, 원본을 갖고 있는 클래스, 제거도 이 곳에서 한다.
/// DX 관련된 핵심 부분을 갖고 있는다. DX에 100% 종속적이다.
/// 렌더링 엔진을 DX로부터 덜 종속적이게 하기위해 만들어짐
/// 렌더링 엔진에 속한다.
/// 2021. 07. 19 정종영
/// </summary>
class DX11Core : public ICore
{
public:
	DX11Core(HWND hWnd);
	virtual ~DX11Core();

private:
	Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContext;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_NormalDSS;

	D3D_FEATURE_LEVEL m_FeatureLevel;		// Status or DirectX Info

	IDXGIFactory* m_pDXGIFactory;
	Microsoft::WRL::ComPtr<IDXGIOutput> m_Output;
	std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> m_AdapterVec;
	DXGI_ADAPTER_DESC1 m_AdapterDesc1;		// 어댑터 정보

private:
	std::unique_ptr<D3D11_VIEWPORT> m_ScreenViewport;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_MainRenderTargetView;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;

public:
	IDXGISwapChain* GetSwapChain() { return m_SwapChain.Get(); }

public:
	virtual bool Initialize(HWND hWnd) override;
	virtual void OnResize() override;

public:
	virtual void ClearMainRenderTarget() override;
	virtual void ClearDepthBuffer() override;
	virtual void SetMainRenderTarget() override;
	virtual void SetMainViewport() override;
	virtual void SetDefaultStates() override;
	virtual void ClearShaderResources() override;
	virtual void Present() override;
	virtual void Debug() override;

public:
	// Device, DC, Viewport ...
	ID3D11Device* GetDevice() { return m_Device.Get(); }
	ID3D11DeviceContext* GetDC() { return m_DeviceContext.Get(); }
	ID3D11DepthStencilState* GetNormalDSS() { return m_NormalDSS.Get(); }
	ID3D11DepthStencilView* GetDSV() { return m_DepthStencilView.Get(); }
	D3D11_VIEWPORT* GetViewport() { return m_ScreenViewport.get(); }

private:
	HRESULT GetAdapterInfo();
};

