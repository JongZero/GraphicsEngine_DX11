#include "DX11Core.h"
#include "RenderStates.h"
#include "Vertex.h"
#include "Effects.h"
#include "ClientSize.h"
#include "DebugManager.h"

#include "ErrChecker.h"
#include "MacroDefine.h"
#include "EColors.h"

#include <assert.h>

const wchar_t* FeatureLevelToString(D3D_FEATURE_LEVEL featureLevel);

DX11Core::DX11Core(HWND hWnd)
	: m_pDXGIFactory(nullptr), m_AdapterVec(), m_Output(nullptr),
	m_SwapChain(nullptr), m_MainRenderTargetView(nullptr), m_DepthStencilView(nullptr),
	m_DepthStencilBuffer(nullptr)
{
	m_ScreenViewport = std::make_unique<D3D11_VIEWPORT>();
}

DX11Core::~DX11Core()
{
	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();

	// Restore all default settings. 기본 세팅으로 복원
	if (m_DeviceContext)
	{
		m_DeviceContext->ClearState();
	}
}

bool DX11Core::Initialize(HWND hWnd)
{
	HRESULT hr = S_OK;

	/// 1. Enumerate Adapters 어뎁터들을 열거하자
	// This topic shows how to use Microsoft DirectX Graphics Infrastructure(DXGI) to enumerate the available graphics adapters on a computer.

	// You generally need to enumerate adapters for these reasons :
	// - 1. To determine how many graphics adapters are installed on a computer.
	// - 2. To help you choose which adapter to use to create a Direct3D device.
	// - 3. To retrieve an IDXGIAdapter object that you can use to retrieve device capabilities.

	// 1-1. Create an IDXGIFactory object by calling the CreateDXGIFactory function.
	CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&m_pDXGIFactory);

	// 1-2. Enumerate through each adapter by calling the IDXGIFactory::EnumAdapters method.
	// If no adapter is available at the specified index, the method returns DXGI_ERROR_NOT_FOUND.
	Microsoft::WRL::ComPtr<IDXGIAdapter> _Adapter;
	for (UINT i = 0; m_pDXGIFactory->EnumAdapters(i, &_Adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		m_AdapterVec.push_back(_Adapter);		// 두 개의 그래픽카드 (외장, 내장)
	}

	// 어댑터의 정보를 얻어오기
	GetAdapterInfo();

	/// 2. Create Device and Swap Chain
	// 2-1. Feature Level 
	// 사용자의 하드웨어가 특정 Feature Level(D3D_FEATURE_LEVEL_9_1, D3D_FEATURE_LEVEL_10_0...)을 지원하지 않는다면 그보다 더 낮은(오래된) 기능 수준으로 내려가서 응용 프로그램을 실행
	D3D_FEATURE_LEVEL MaxSupportedFeatureLevel = D3D_FEATURE_LEVEL_9_1;
	D3D_FEATURE_LEVEL FeatureLevels[] = {
		//D3D_FEATURE_LEVEL_11_1,		// 용 책은 11_0 기준으로 만들어짐, msdn에는 11_4 까지 있음 -> 두 가지 모두에 호환하기 위해 11_0 기준으로 생성
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// 2-2. Define initial parameters for a swap chain.
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));						// DXGI_SWAP_CHAIN_DESC 구조체 초기화
	sd.BufferDesc.Width = ClientSize::GetWidth();				// 버퍼의 너비
	sd.BufferDesc.Height = ClientSize::GetHeight();				// 버퍼의 높이
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 백버퍼의 픽셀 형식
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;		// 용책 예제에서 가져온 부분
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;						// 용책 예제에서 가져온 부분
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 버퍼의 용도를 서술, DXGI_USAGE_RENDER_TARGET_OUTPUT : 후면 버퍼에 렌더링을 한다.(즉, 버퍼를 렌더 대상으로 사용한다.)
	sd.BufferCount = 1;									// Swap Chain 에서 사용할 후면 버퍼의 갯수

	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	sd.OutputWindow = hWnd;								// 렌더링 결과를 표시할 창의 핸들
	sd.Windowed = TRUE;									// TRUE : 창모드, FALSE : 전체화면
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			// 용책 예제에서 가져온 부분
	sd.Flags = 0;		// 용책 예제에서 가져온 부분

	// 2-3. Create the SwapChain by calling CreateSwapChain.
	D3D_FEATURE_LEVEL returnedFeatureLevel;
	UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;		// 디버그 모드 빌드에서 디버그 계층을 활성화함, (DX가 VC++ 출력 창에 디버그 메시지를 보냄)
#endif

	/// D2D Engine 사용을 위해 BGRA 지원하도록 설정
	createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 
		FeatureLevels, 1, D3D11_SDK_VERSION, &sd, m_SwapChain.GetAddressOf(), m_Device.GetAddressOf(), 
		&returnedFeatureLevel, m_DeviceContext.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBox(0, L"Create Device and SwapChain Failed", 0, 0);
		return false;
	}

	if (returnedFeatureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 Unsupported", 0, 0);
		return false;
	}

	// 리턴 받은 FeatureLevel 정보를 저장
	m_FeatureLevel = returnedFeatureLevel;

	ReleaseCOM(m_pDXGIFactory);

	/// 3. Get Adapter Display Modes
	// This topic shows how to use Microsoft DirectX Graphics Infrastructure(DXGI) to get the valid display modes associated with an adapter.

	// 3-1. Create an IDXGIFactory object and use it to enumerate the available adapters.
	// -> 위 2번에서 함

	// 3-2. Call IDXGIAdapter::EnumOutputs to enumerate the outputs for each adapter.
	// Adapter[0] : 외장그래픽카드 Adapter, [1] : 내장그래픽카드 Adapter
	hr = m_AdapterVec[0]->EnumOutputs(0, m_Output.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBox(0, L"EnumOutputs Failed", 0, 0);
		return false;
	}

	// 3-3. Call IDXGIOutput::GetDisplayModeList to retrieve an array of DXGI_MODE_DESC structures and the number of elements in the array.
	UINT numModes = 0;
	DXGI_MODE_DESC* displayModes = nullptr;
	DXGI_FORMAT format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	// Get the number of elements
	hr = m_Output->GetDisplayModeList(format, 0, &numModes, NULL);

	if (FAILED(hr))
	{
		MessageBox(0, L"GetDisplayModeList Failed", 0, 0);
		return false;
	}

	displayModes = new DXGI_MODE_DESC[numModes];

	// Get the list
	hr = m_Output->GetDisplayModeList(format, 0, &numModes, displayModes);

	if (FAILED(hr))
	{
		MessageBox(0, L"GetDisplayModeList Failed", 0, 0);
		return false;
	}

	delete[] displayModes;
	displayModes = nullptr;

	// 폰트용 DSS 생성
	ID3D11DepthStencilState* normalDSS = nullptr;

	D3D11_DEPTH_STENCIL_DESC equalsDesc;
	ZeroMemory(&equalsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	equalsDesc.DepthEnable = true;
	equalsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;		// 깊이버퍼에 쓰기는 한다
	equalsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	HR(m_Device->CreateDepthStencilState(&equalsDesc, &normalDSS));

	// Must init Effects first since InputLayouts depend on shader signatures.
	RenderStates::InitAll(m_Device.Get());
	Effects::InitAll(m_Device.Get());
	InputLayouts::InitAll(m_Device.Get());

	OnResize();

	return true;
}

void DX11Core::OnResize()
{
	assert(m_Device);
	assert(m_DeviceContext);
	assert(m_SwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.
	m_MainRenderTargetView = nullptr;
	m_DepthStencilView = nullptr;
	m_DepthStencilBuffer = nullptr;

	// Resize the swap chain and Main RenderTarget
	HR(m_SwapChain->ResizeBuffers(1, ClientSize::GetWidth(), ClientSize::GetHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	ID3D11Texture2D* backBuffer;
	HR(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));	// SwapChain을 가리키는 포인터를 얻는다. 3번째 매개변수는 백버퍼를 가리키는 포인터를 돌려줌

	/// 5. recreate the render target view.
	HR(m_Device->CreateRenderTargetView(backBuffer, 0, m_MainRenderTargetView.GetAddressOf()));
	ReleaseCOM(backBuffer);	// GetBuffer를 호출하면 백 버퍼에 대한 COM 참조 횟수(reference count)가 증가한다. 그러므로 백 버퍼를 Release 해준다.

	/// 6. Create a Depth·Stencil Buffer 깊이·스텐실 버퍼와 뷰 생성
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = ClientSize::GetWidth();						// 텍스처의 너비(텍셀 단위)
	depthStencilDesc.Height = ClientSize::GetHeight();					// 텍스처의 높이(텍셀 단위)
	depthStencilDesc.MipLevels = 1;								// 밉맵 수준의 갯수, 깊이·스텐실 버퍼를 위한 텍스처에서는 밉맵 수준이 하나만 있으면 된다.
	depthStencilDesc.ArraySize = 1;								// 텍스처 배열의 텍스처 갯수, 깊이·스텐실 버퍼의 경우에는 텍스처 하나만 필요하다.
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 텍셀의 형식, 깊이·스텐실 버퍼의 경우 용책 p109의 형식 중 하나로 지정해야한다. (자세한 내용 p109 참고)

	depthStencilDesc.SampleDesc.Count = 1;				// SampleDesc : 다중 표본 갯수와 품질 수준을 서술하는 구조체, 4X MSAA는 화면 해상도의 4배 크기인 후면 버퍼 하나와 깊이 버퍼 하나를 사용해서 부분픽셀당 색상과 깊이·스텐실 정보를 담는다.
	depthStencilDesc.SampleDesc.Quality = 0;			// 따라서 깊이·스텐실 버퍼를 위한 다중표본화 설정은 렌더 타겟에 쓰인 설정과 일치해야 한다.

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;			// 텍스처의 용도, D3D11_USAGE_DEFAULT : 자원을 GPU가 읽고 써야 한다면 이 용도를 설정한다. 이 용도를 설정하면 CPU는 자원을 읽거나 쓸 수 없다. 
															// 깊이·스텐실 버퍼에 대한 모든 읽기, 쓰기는 GPU가 수행하므로 깊이·스텐실 버퍼를 위한 텍스처를 생성할 때에는 바로 이 D3D11_USAGE_DEFAULT를 사용한다.
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;	// 자원을 파이프라인에 어떤 식으로 묶을 것인지를 지정하는 하나 이상의 플래그들을 OR로 결합해서 지정한다. 깊이·스텐실 버퍼의 경우 D3D11_BIND_DEPTH_STENCIL 플래그를 지정해야 한다.
	depthStencilDesc.CPUAccessFlags = 0;					// CPU가 자원에 접근하는 방식을 결정하는 플래그들을 지정한다. 깊이·스텐실 버퍼의 경우 GPU만 깊이·스텐실 버퍼를 읽고 쓸 뿐 CPU는 전혀 접근하지 않으므로 0을 지정
	depthStencilDesc.MiscFlags = 0;							// 기타 플래그들, 깊이·스텐실 버퍼에는 적용되지 않으므로 그냥 0 지정

	HR(m_Device->CreateTexture2D(&depthStencilDesc,
		0,					// 텍스처에 채울 초기 자료를 가리키는 포인터, 이 텍스처는 깊이·스텐실 버퍼로 사용할 것이므로 따로 자료를 채울 필요 X
		m_DepthStencilBuffer.GetAddressOf()));
	HR(m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), 0, m_DepthStencilView.GetAddressOf()));

	/// 7. 뷰들을 출력 병합기 단계에 묶기
	m_DeviceContext->OMSetRenderTargets(1,						// 묶고자 하는 렌더 타겟의 갯수
		m_MainRenderTargetView.GetAddressOf(),		// 파이프라인에 묶을 렌더 대상 뷰들을 가리키는 포인터들을 담은 배열의 첫 원소를 가리키는 포인터
		m_DepthStencilView.Get());

	/// 8. Set the viewport transform
	m_ScreenViewport->TopLeftX = 0.0f;
	m_ScreenViewport->TopLeftY = 0.0f;
	m_ScreenViewport->Width = static_cast<float>(ClientSize::GetWidth());
	m_ScreenViewport->Height = static_cast<float>(ClientSize::GetHeight());
	m_ScreenViewport->MinDepth = 0.0f;
	m_ScreenViewport->MaxDepth = 1.0f;

	m_DeviceContext->RSSetViewports(1, m_ScreenViewport.get());
}

void DX11Core::ClearMainRenderTarget()
{
	m_DeviceContext->ClearRenderTargetView(m_MainRenderTargetView.Get(), reinterpret_cast<const float*>(&EColors::Black));
	SetMainViewport();
}

void DX11Core::ClearDepthBuffer()
{
	// 깊이 버퍼를 1.0f로, 스텐실 버퍼를 0으로 지운다.
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//m_pDeviceContext->OMSetDepthStencilState(m_pNormalDSS.Get(), 0);
}

void DX11Core::SetMainRenderTarget()
{
	m_DeviceContext->RSSetState(0);

	// 원래의 백버퍼와 깊이 버퍼를 다시 출력 병합기 단계에 묶는다.
	m_DeviceContext->OMSetRenderTargets(1, m_MainRenderTargetView.GetAddressOf(), m_DepthStencilView.Get());
	SetMainViewport();
}

void DX11Core::SetMainViewport()
{
	m_DeviceContext->RSSetViewports(1, m_ScreenViewport.get());
}

void DX11Core::SetDefaultStates()
{
	// restore default states, as the SkyFX changes them in the effect file.
	// 폰트를 위해서도 초기화해줘야한다.
	m_DeviceContext->RSSetState(0);
	m_DeviceContext->OMSetDepthStencilState(0, 0);
}

void DX11Core::ClearShaderResources()
{
	// Unbind shadow map as a shader input because we are going to render to it next frame.
	// The shadow might might be at any slot, so clear all slots.
	// 이 코드가 끝난 후 다음 프레임으로 렌더할 예정이므로 쉐이더 인풋으로써의 쉐도우 맵을 언바인드 해야한다.
	// 쉐도우맵이 임의의 슬롯에 있을 수 있으므로, 모든 슬롯을 클리어한다. -> 이를 안하면 경고 메시지가 뜬다.
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	m_DeviceContext->PSSetShaderResources(0, 16, nullSRV);
}

void DX11Core::Present()
{
	m_SwapChain->Present(0, 0);
}

void DX11Core::Debug()
{
	// 어댑터 정보
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, D2D1::ColorF::Red, (TCHAR*)L"[ Adapter Info ]"));
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, (TCHAR*)L"Feature Level : %s", FeatureLevelToString(m_FeatureLevel)));
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, (TCHAR*)L"Description: %s", m_AdapterDesc1.Description));
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, (TCHAR*)L"VendorID: %u", m_AdapterDesc1.VendorId));
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, (TCHAR*)L"DeviceID: %u", m_AdapterDesc1.DeviceId));
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, (TCHAR*)L"SubSysID: %u", m_AdapterDesc1.SubSysId));
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, (TCHAR*)L"Revision: %u", m_AdapterDesc1.Revision));
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, (TCHAR*)L"VideoMemory: %lu MB", m_AdapterDesc1.DedicatedVideoMemory / 1024 / 1024));
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, (TCHAR*)L"SystemMemory: %lu MB", m_AdapterDesc1.DedicatedSystemMemory / 1024 / 1024));
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, (TCHAR*)L"AdpaterLuid: %u.%d", m_AdapterDesc1.AdapterLuid.HighPart, m_AdapterDesc1.AdapterLuid.LowPart));
}

HRESULT DX11Core::GetAdapterInfo()
{
	// 디바이스/GPU 정보 획득 <방법 #1> 
	// DXGI 1.1 / DXGI Factory 사용 / DX11 이상시 권장, DXGI 1.0 과 1.1 혼용금지. DXGI 1.3 은 Windows 8.1 이상필수.

	// DXGI 1.1 대응
	// DXGI버전별로 다름
	IDXGIAdapter1* pAdapter;
	IDXGIFactory1* pFactory = NULL;

	HRESULT hr = S_OK;

	// DXGIFactory 개체 생성(DXGI.lib 필요)
	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory)))
	{
		return E_FAIL;
	}

	hr = pFactory->
		EnumAdapters1(0, &pAdapter);

	if (hr != DXGI_ERROR_NOT_FOUND)
	{
		pAdapter->GetDesc1(&m_AdapterDesc1);
	}

	ReleaseCOM(pAdapter);
	ReleaseCOM(pFactory);

	return S_OK;
}

const wchar_t* FeatureLevelToString(D3D_FEATURE_LEVEL featureLevel)
{
	switch (featureLevel)
	{
	case D3D_FEATURE_LEVEL_1_0_CORE:
		return L"D3D_FEATURE_LEVEL_1_0_CORE";
		break;
	case D3D_FEATURE_LEVEL_9_1:
		return L"D3D_FEATURE_LEVEL_9_1";
		break;
	case D3D_FEATURE_LEVEL_9_2:
		return L"D3D_FEATURE_LEVEL_9_2";
		break;
	case D3D_FEATURE_LEVEL_9_3:
		return L"D3D_FEATURE_LEVEL_9_3";
		break;
	case D3D_FEATURE_LEVEL_10_0:
		return L"D3D_FEATURE_LEVEL_10_0";
		break;
	case D3D_FEATURE_LEVEL_10_1:
		return L"D3D_FEATURE_LEVEL_10_1";
		break;
	case D3D_FEATURE_LEVEL_11_0:
		return L"D3D_FEATURE_LEVEL_11_0";
		break;
	case D3D_FEATURE_LEVEL_11_1:
		return L"D3D_FEATURE_LEVEL_11_1";
		break;
	case D3D_FEATURE_LEVEL_12_0:
		return L"D3D_FEATURE_LEVEL_12_0";
		break;
	case D3D_FEATURE_LEVEL_12_1:
		return L"D3D_FEATURE_LEVEL_12_1";
		break;
	default:
		return L"Invalid FEATURE_LEVEL";
		break;
	}
}
