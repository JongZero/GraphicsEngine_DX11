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

	// Restore all default settings. �⺻ �������� ����
	if (m_DeviceContext)
	{
		m_DeviceContext->ClearState();
	}
}

bool DX11Core::Initialize(HWND hWnd)
{
	HRESULT hr = S_OK;

	/// 1. Enumerate Adapters ��͵��� ��������
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
		m_AdapterVec.push_back(_Adapter);		// �� ���� �׷���ī�� (����, ����)
	}

	// ������� ������ ������
	GetAdapterInfo();

	/// 2. Create Device and Swap Chain
	// 2-1. Feature Level 
	// ������� �ϵ��� Ư�� Feature Level(D3D_FEATURE_LEVEL_9_1, D3D_FEATURE_LEVEL_10_0...)�� �������� �ʴ´ٸ� �׺��� �� ����(������) ��� �������� �������� ���� ���α׷��� ����
	D3D_FEATURE_LEVEL MaxSupportedFeatureLevel = D3D_FEATURE_LEVEL_9_1;
	D3D_FEATURE_LEVEL FeatureLevels[] = {
		//D3D_FEATURE_LEVEL_11_1,		// �� å�� 11_0 �������� �������, msdn���� 11_4 ���� ���� -> �� ���� ��ο� ȣȯ�ϱ� ���� 11_0 �������� ����
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// 2-2. Define initial parameters for a swap chain.
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));						// DXGI_SWAP_CHAIN_DESC ����ü �ʱ�ȭ
	sd.BufferDesc.Width = ClientSize::GetWidth();				// ������ �ʺ�
	sd.BufferDesc.Height = ClientSize::GetHeight();				// ������ ����
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// ������� �ȼ� ����
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;		// ��å �������� ������ �κ�
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;						// ��å �������� ������ �κ�
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// ������ �뵵�� ����, DXGI_USAGE_RENDER_TARGET_OUTPUT : �ĸ� ���ۿ� �������� �Ѵ�.(��, ���۸� ���� ������� ����Ѵ�.)
	sd.BufferCount = 1;									// Swap Chain ���� ����� �ĸ� ������ ����

	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	sd.OutputWindow = hWnd;								// ������ ����� ǥ���� â�� �ڵ�
	sd.Windowed = TRUE;									// TRUE : â���, FALSE : ��üȭ��
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;			// ��å �������� ������ �κ�
	sd.Flags = 0;		// ��å �������� ������ �κ�

	// 2-3. Create the SwapChain by calling CreateSwapChain.
	D3D_FEATURE_LEVEL returnedFeatureLevel;
	UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;		// ����� ��� ���忡�� ����� ������ Ȱ��ȭ��, (DX�� VC++ ��� â�� ����� �޽����� ����)
#endif

	/// D2D Engine ����� ���� BGRA �����ϵ��� ����
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

	// ���� ���� FeatureLevel ������ ����
	m_FeatureLevel = returnedFeatureLevel;

	ReleaseCOM(m_pDXGIFactory);

	/// 3. Get Adapter Display Modes
	// This topic shows how to use Microsoft DirectX Graphics Infrastructure(DXGI) to get the valid display modes associated with an adapter.

	// 3-1. Create an IDXGIFactory object and use it to enumerate the available adapters.
	// -> �� 2������ ��

	// 3-2. Call IDXGIAdapter::EnumOutputs to enumerate the outputs for each adapter.
	// Adapter[0] : ����׷���ī�� Adapter, [1] : ����׷���ī�� Adapter
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

	// ��Ʈ�� DSS ����
	ID3D11DepthStencilState* normalDSS = nullptr;

	D3D11_DEPTH_STENCIL_DESC equalsDesc;
	ZeroMemory(&equalsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	equalsDesc.DepthEnable = true;
	equalsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;		// ���̹��ۿ� ����� �Ѵ�
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
	HR(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));	// SwapChain�� ����Ű�� �����͸� ��´�. 3��° �Ű������� ����۸� ����Ű�� �����͸� ������

	/// 5. recreate the render target view.
	HR(m_Device->CreateRenderTargetView(backBuffer, 0, m_MainRenderTargetView.GetAddressOf()));
	ReleaseCOM(backBuffer);	// GetBuffer�� ȣ���ϸ� �� ���ۿ� ���� COM ���� Ƚ��(reference count)�� �����Ѵ�. �׷��Ƿ� �� ���۸� Release ���ش�.

	/// 6. Create a Depth��Stencil Buffer ���̡����ٽ� ���ۿ� �� ����
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = ClientSize::GetWidth();						// �ؽ�ó�� �ʺ�(�ؼ� ����)
	depthStencilDesc.Height = ClientSize::GetHeight();					// �ؽ�ó�� ����(�ؼ� ����)
	depthStencilDesc.MipLevels = 1;								// �Ӹ� ������ ����, ���̡����ٽ� ���۸� ���� �ؽ�ó������ �Ӹ� ������ �ϳ��� ������ �ȴ�.
	depthStencilDesc.ArraySize = 1;								// �ؽ�ó �迭�� �ؽ�ó ����, ���̡����ٽ� ������ ��쿡�� �ؽ�ó �ϳ��� �ʿ��ϴ�.
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// �ؼ��� ����, ���̡����ٽ� ������ ��� ��å p109�� ���� �� �ϳ��� �����ؾ��Ѵ�. (�ڼ��� ���� p109 ����)

	depthStencilDesc.SampleDesc.Count = 1;				// SampleDesc : ���� ǥ�� ������ ǰ�� ������ �����ϴ� ����ü, 4X MSAA�� ȭ�� �ػ��� 4�� ũ���� �ĸ� ���� �ϳ��� ���� ���� �ϳ��� ����ؼ� �κ��ȼ��� ����� ���̡����ٽ� ������ ��´�.
	depthStencilDesc.SampleDesc.Quality = 0;			// ���� ���̡����ٽ� ���۸� ���� ����ǥ��ȭ ������ ���� Ÿ�ٿ� ���� ������ ��ġ�ؾ� �Ѵ�.

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;			// �ؽ�ó�� �뵵, D3D11_USAGE_DEFAULT : �ڿ��� GPU�� �а� ��� �Ѵٸ� �� �뵵�� �����Ѵ�. �� �뵵�� �����ϸ� CPU�� �ڿ��� �аų� �� �� ����. 
															// ���̡����ٽ� ���ۿ� ���� ��� �б�, ����� GPU�� �����ϹǷ� ���̡����ٽ� ���۸� ���� �ؽ�ó�� ������ ������ �ٷ� �� D3D11_USAGE_DEFAULT�� ����Ѵ�.
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;	// �ڿ��� ���������ο� � ������ ���� �������� �����ϴ� �ϳ� �̻��� �÷��׵��� OR�� �����ؼ� �����Ѵ�. ���̡����ٽ� ������ ��� D3D11_BIND_DEPTH_STENCIL �÷��׸� �����ؾ� �Ѵ�.
	depthStencilDesc.CPUAccessFlags = 0;					// CPU�� �ڿ��� �����ϴ� ����� �����ϴ� �÷��׵��� �����Ѵ�. ���̡����ٽ� ������ ��� GPU�� ���̡����ٽ� ���۸� �а� �� �� CPU�� ���� �������� �����Ƿ� 0�� ����
	depthStencilDesc.MiscFlags = 0;							// ��Ÿ �÷��׵�, ���̡����ٽ� ���ۿ��� ������� �����Ƿ� �׳� 0 ����

	HR(m_Device->CreateTexture2D(&depthStencilDesc,
		0,					// �ؽ�ó�� ä�� �ʱ� �ڷḦ ����Ű�� ������, �� �ؽ�ó�� ���̡����ٽ� ���۷� ����� ���̹Ƿ� ���� �ڷḦ ä�� �ʿ� X
		m_DepthStencilBuffer.GetAddressOf()));
	HR(m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), 0, m_DepthStencilView.GetAddressOf()));

	/// 7. ����� ��� ���ձ� �ܰ迡 ����
	m_DeviceContext->OMSetRenderTargets(1,						// ������ �ϴ� ���� Ÿ���� ����
		m_MainRenderTargetView.GetAddressOf(),		// ���������ο� ���� ���� ��� ����� ����Ű�� �����͵��� ���� �迭�� ù ���Ҹ� ����Ű�� ������
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
	// ���� ���۸� 1.0f��, ���ٽ� ���۸� 0���� �����.
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//m_pDeviceContext->OMSetDepthStencilState(m_pNormalDSS.Get(), 0);
}

void DX11Core::SetMainRenderTarget()
{
	m_DeviceContext->RSSetState(0);

	// ������ ����ۿ� ���� ���۸� �ٽ� ��� ���ձ� �ܰ迡 ���´�.
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
	// ��Ʈ�� ���ؼ��� �ʱ�ȭ������Ѵ�.
	m_DeviceContext->RSSetState(0);
	m_DeviceContext->OMSetDepthStencilState(0, 0);
}

void DX11Core::ClearShaderResources()
{
	// Unbind shadow map as a shader input because we are going to render to it next frame.
	// The shadow might might be at any slot, so clear all slots.
	// �� �ڵ尡 ���� �� ���� ���������� ������ �����̹Ƿ� ���̴� ��ǲ���ν��� ������ ���� ����ε� �ؾ��Ѵ�.
	// ��������� ������ ���Կ� ���� �� �����Ƿ�, ��� ������ Ŭ�����Ѵ�. -> �̸� ���ϸ� ��� �޽����� ���.
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	m_DeviceContext->PSSetShaderResources(0, 16, nullSRV);
}

void DX11Core::Present()
{
	m_SwapChain->Present(0, 0);
}

void DX11Core::Debug()
{
	// ����� ����
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
	// ����̽�/GPU ���� ȹ�� <��� #1> 
	// DXGI 1.1 / DXGI Factory ��� / DX11 �̻�� ����, DXGI 1.0 �� 1.1 ȥ�����. DXGI 1.3 �� Windows 8.1 �̻��ʼ�.

	// DXGI 1.1 ����
	// DXGI�������� �ٸ�
	IDXGIAdapter1* pAdapter;
	IDXGIFactory1* pFactory = NULL;

	HRESULT hr = S_OK;

	// DXGIFactory ��ü ����(DXGI.lib �ʿ�)
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
