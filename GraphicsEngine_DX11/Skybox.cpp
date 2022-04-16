#include "Skybox.h"
#include "DX11Core.h"
#include "MeshBuilder.h"
#include "ResourceManager.h"
#include "RenderTarget.h"
#include "Vertex.h"
#include "Effects.h"
#include "EColors.h"
#include "Shared_RenderingData.h"
#include "CameraUtility.h"
#include "Mesh.h"
#include "ClientSize.h"
#include "DebugManager.h"
#include "Cubemap.h"
#include "RenderStates.h"

#include <assert.h>

Skybox::Skybox(std::shared_ptr<DX11Core> dx11Core, MeshBuilder* pMeshBuilder, std::shared_ptr<ResourceManager> rm)
	: m_DX11Core(dx11Core), m_ResourceManager(rm), m_pBrdfLUT(nullptr), m_pCubemap(nullptr),
	m_pIrradianceMap(nullptr), m_pPreFilterMap(nullptr)
{
	ObjectMeshInfo omi(ObjectMeshInfo::eMeshType::Cube,
		ObjectMeshInfo::eRenderStateType::Solid, ObjectMeshInfo::eEffectType::Basic);
	pMeshBuilder->CreateMeshes(L"Skybox", omi);

	m_pSkyboxMeshData = rm->GetObjectMeshData(L"Skybox");
}

Skybox::~Skybox()
{
	// SkyboxMeshData의 소유권은 리소스 매니저에 있음
	// 그 곳에서 해제시킬 것이므로 여기서 해제 X
}

void Skybox::Initialize()
{
	ID3D11Device* _device = m_DX11Core->GetDevice();
	//_pFrameBuffer = frameBuffer;
	//_pCamera = camera;

	///CreateCubeMap(d3d, hwnd);

	std::vector<RenderTarget*> _cubeFaces;
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	// Load initial image
	ID3D11ShaderResourceView* _skyCubeMap = m_ResourceManager->GetSkyCubeMap(L"environment");

	const int _SkyboxSize = 2048;
	const int _IrradianceSize = 32;
	const int _PreFilterSize = 256;
	const int _BrdfLookupSize = 512;

	// Create faces
	for (int i = 0; i < 6; i++)
	{
		RenderTarget* _renderTarget = new RenderTarget(_SkyboxSize, _SkyboxSize, L"renderTarget 0");

		_renderTarget->OnResize(m_DX11Core->GetDevice(), _SkyboxSize, _SkyboxSize);
		_cubeFaces.push_back(_renderTarget);
	}

	// Rect to Cubemap Shader
	
	// BindMesh
	unsigned int _stride = sizeof(Vertex::PosNormalTex);
	unsigned int _offset = 0;


	_dc->RSSetState(RenderStates::GetSolidRS());
	_dc->IASetInputLayout(InputLayouts::PosNormalTex);


	_dc->IASetVertexBuffers(0, 1, m_pSkyboxMeshData->m_VertexBuffer.GetAddressOf(), &m_pSkyboxMeshData->m_Stride, &_offset);
	_dc->IASetIndexBuffer(m_pSkyboxMeshData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	ID3DX11EffectTechnique* _activeTech = Effects::SkyFX->RectToCubeMapTech;
	//const float lastFov = _pCamera->GetFOV();
	//const float lastAspect = _pCamera->GetAspectRatio();

	const float fov = ConvertToRadians(90.0f);
	const float aspectRatio = 1.0f;



	/// Render
	CameraUtility _camera;

	// cache properties
	//mFovY = fovY;
	//mAspect = aspect;
	//mNearZ = zn;
	//mFarZ = zf;



	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof depthBufferDesc);

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = _SkyboxSize;
	depthBufferDesc.Height = _SkyboxSize;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	ID3D11Texture2D* _pDepthStencilBuffer;

	// Create the texture for the depth buffer using the filled out description.
	HRESULT result = _device->CreateTexture2D(&depthBufferDesc, nullptr, &_pDepthStencilBuffer);
	assert(_pDepthStencilBuffer);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	ID3D11DepthStencilState* _pDepthStencilState;
	// Create the depth stencil state.
	result = _device->CreateDepthStencilState(&depthStencilDesc, &_pDepthStencilState);

	// Set the depth stencil state.
	_dc->OMSetDepthStencilState(_pDepthStencilState, 1);

	// Initailze the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof depthStencilViewDesc);

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	ID3D11DepthStencilView* _pDepthStencilView;

	// Create the depth stencil view.
	result = _device->CreateDepthStencilView(_pDepthStencilBuffer, &depthStencilViewDesc, &_pDepthStencilView);
	assert(_pDepthStencilView);


	D3D11_VIEWPORT viewport;
	viewport.Width = float(_SkyboxSize);
	viewport.Height = float(_SkyboxSize);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;


	EMath::Matrix _proj = _camera.MatrixPerspectiveFovLH(fov, aspectRatio, 0.1f, 1000.0f);

	for (int i = 0; i < 6; i++)
	{
		RenderTarget* _texture = _cubeFaces[i];

		ID3D11RenderTargetView* rt[1] = { _texture->GetRTV() };
		_dc->OMSetRenderTargets(1, rt, _pDepthStencilView);
		_dc->RSSetViewports(1, &viewport);


		_dc->ClearRenderTargetView(_texture->GetRTV(), reinterpret_cast<const float*>(&EColors::Yellow));
		// Clear the depth buffer.
		_dc->ClearDepthStencilView(_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


		EMath::Vector3 _rot;
		if (i == 0) _rot = EMath::Vector3(0.0f, 90.0f, 0.0f); // front
		else if (i == 1) _rot = EMath::Vector3(0.0f, 270.0f, 0.0f); // back
		else if (i == 2) _rot = EMath::Vector3(-90.0f, 0.0f, 0.0f); // top
		else if (i == 3) _rot = EMath::Vector3(90.0f, 0.0f, 0.0f); // bottom
		else if (i == 4) _rot = EMath::Vector3(0.0f, 0.0f, 0.0f); // left
		else if (i == 5) _rot = EMath::Vector3(0.0f, 180.0f, 0.0f); // right

		//m_SharedRenderingData->m_View
		EMath::Vector3 _initPos(0, 0, 10);
		EMath::Vector3 _lookAt(0, 0, -1);
		EMath::Matrix _view = _camera.UpdateSkycubeViewMatrix(_initPos, _lookAt, _rot);

		Effects::SkyFX->SetTexture(_skyCubeMap);
		Effects::SkyFX->SetView(_view);
		Effects::SkyFX->SetProj(_proj);

		D3DX11_TECHNIQUE_DESC techDesc;
		_activeTech->GetDesc(&techDesc);

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			_activeTech->GetPassByIndex(p)->Apply(0, _dc);

			_dc->DrawIndexed(m_pSkyboxMeshData->m_MeshVec[0]->IndexCount,
				m_pSkyboxMeshData->m_MeshVec[0]->StartIndex,
				0);
		}

		DebugManager::GetInstance()->AddRenderTarget(_texture);
	}
	
	m_pCubemap = std::make_unique<Cubemap>(m_DX11Core.get(), _cubeFaces, _SkyboxSize, _SkyboxSize, 1);

	std::vector<RenderTarget*> _cubeFaces_IrradianceSize(6);
	for (int i = 0; i < 6; ++i)
	{
		RenderTarget* _renderTexture = new RenderTarget(_IrradianceSize, _IrradianceSize);
		_renderTexture->OnResize(m_DX11Core->GetDevice(), _IrradianceSize, _IrradianceSize);
		_cubeFaces_IrradianceSize[i] = _renderTexture;
	}

	ID3D11ShaderResourceView* _cubemapSRV = m_pCubemap->GetSRV();
	Effects::SkyFX->SetCubeMap(_cubemapSRV);

	_activeTech = Effects::SkyFX->IrradianceTech;

	// Render
	for (int i = 0; i < 6; ++i)
	{
		RenderTarget* _texture = _cubeFaces_IrradianceSize[i];

		_dc->ClearRenderTargetView(_texture->GetRTV(), reinterpret_cast<const float*>(&EColors::SpringGreen));

		ID3D11RenderTargetView* rt[1] = { _texture->GetRTV() };
		_dc->OMSetRenderTargets(1, rt, 0);

		EMath::Vector3 _rot;
		if (i == 0) _rot = EMath::Vector3(0.0f, 90.0f, 0.0f); // front
		else if (i == 1) _rot = EMath::Vector3(0.0f, 270.0f, 0.0f); // back
		else if (i == 2) _rot = EMath::Vector3(-90.0f, 0.0f, 0.0f); // top
		else if (i == 3) _rot = EMath::Vector3(90.0f, 0.0f, 0.0f); // bottom
		else if (i == 4) _rot = EMath::Vector3(0.0f, 0.0f, 0.0f); // left
		else if (i == 5) _rot = EMath::Vector3(0.0f, 180.0f, 0.0f); // right

		EMath::Vector3 _initPos(0, 0, 10);
		EMath::Vector3 _lookAt(0, 0, -1);
		EMath::Matrix _view = _camera.UpdateSkycubeViewMatrix(_initPos, _lookAt, _rot);

		Effects::SkyFX->SetView(_view);

		D3DX11_TECHNIQUE_DESC techDesc;
		_activeTech->GetDesc(&techDesc);

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			_activeTech->GetPassByIndex(p)->Apply(0, _dc);

			_dc->DrawIndexed(m_pSkyboxMeshData->m_MeshVec[0]->IndexCount,
				m_pSkyboxMeshData->m_MeshVec[0]->StartIndex,
				0);
		}
	}

	m_pIrradianceMap = std::make_unique<Cubemap>(m_DX11Core.get(), _cubeFaces_IrradianceSize, _IrradianceSize, _IrradianceSize, 1);
	m_pPreFilterMap = std::make_unique<Cubemap>(m_DX11Core.get(), std::vector<RenderTarget*>(), _PreFilterSize, _PreFilterSize, 5);

	_activeTech = Effects::SkyFX->PreFilterTech;

	// Render
	for (int mip = 0; mip < 5; ++mip)
	{
		const unsigned int mipWidth = unsigned int(_PreFilterSize * pow(0.5, mip));
		const unsigned int mipHeight = unsigned int(_PreFilterSize * pow(0.5, mip));

		for (int i = 0; i < 6; ++i)
		{
			delete _cubeFaces_IrradianceSize[i];
			RenderTarget* renderTexture = new RenderTarget(mipWidth, mipHeight);
			renderTexture->OnResize(m_DX11Core->GetDevice(), mipWidth, mipHeight);
			_cubeFaces_IrradianceSize[i] = renderTexture;
		}

		const float roughness = float(mip) / 4.0f;
		Effects::SkyFX->SetRoughness(roughness);

		for (int i = 0; i < 6; ++i)
		{
			RenderTarget* _texture = _cubeFaces_IrradianceSize[i];

			_dc->ClearRenderTargetView(_texture->GetRTV(), reinterpret_cast<const float*>(&EColors::SpringGreen));

			ID3D11RenderTargetView* rt[1] = { _texture->GetRTV() };
			_dc->OMSetRenderTargets(1, rt, 0);

			EMath::Vector3 _rot;
			if (i == 0) _rot = EMath::Vector3(0.0f, 90.0f, 0.0f); // front
			else if (i == 1) _rot = EMath::Vector3(0.0f, 270.0f, 0.0f); // back
			else if (i == 2) _rot = EMath::Vector3(-90.0f, 0.0f, 0.0f); // top
			else if (i == 3) _rot = EMath::Vector3(90.0f, 0.0f, 0.0f); // bottom
			else if (i == 4) _rot = EMath::Vector3(0.0f, 0.0f, 0.0f); // left
			else if (i == 5) _rot = EMath::Vector3(0.0f, 180.0f, 0.0f); // right

			EMath::Vector3 _initPos(0, 0, 10);
			EMath::Vector3 _lookAt(0, 0, -1);
			EMath::Matrix _view = _camera.UpdateSkycubeViewMatrix(_initPos, _lookAt, _rot);

			Effects::SkyFX->SetView(_view);

			D3DX11_TECHNIQUE_DESC techDesc;
			_activeTech->GetDesc(&techDesc);

			for (UINT p = 0; p < techDesc.Passes; ++p)
			{
				_activeTech->GetPassByIndex(p)->Apply(0, _dc);

				_dc->DrawIndexed(m_pSkyboxMeshData->m_MeshVec[0]->IndexCount,
					m_pSkyboxMeshData->m_MeshVec[0]->StartIndex,
					0);
			}
		}

		m_pPreFilterMap->Copy(m_DX11Core.get(), _cubeFaces_IrradianceSize, mipWidth, mipHeight, mip);
	}

	for (int i = 0; i < 6; ++i)
	{
		delete _cubeFaces_IrradianceSize[i];
	}

	m_pBrdfLUT = std::make_unique<RenderTarget>(_BrdfLookupSize, _BrdfLookupSize, L"BRDF LUT");
	m_pBrdfLUT->OnResize(m_DX11Core->GetDevice(), _BrdfLookupSize, _BrdfLookupSize);

	_dc->ClearRenderTargetView(m_pBrdfLUT->GetRTV(), reinterpret_cast<const float*>(&EColors::Red));

	ID3D11RenderTargetView* rt[1] = { m_pBrdfLUT->GetRTV() };
	_dc->OMSetRenderTargets(1, rt, 0);

	EMath::Vector3 _rot(0, 0, 0);
	EMath::Vector3 _initPos(0, 0, 10);
	EMath::Vector3 _lookAt(0, 0, -1);
	EMath::Matrix _view = _camera.UpdateSkycubeViewMatrix(_initPos, _lookAt, _rot);
	Effects::SkyFX->SetView(_view);

	_activeTech = Effects::SkyFX->IntegrateBRDFTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	_activeTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		_activeTech->GetPassByIndex(p)->Apply(0, _dc);

		_dc->DrawIndexed(m_pSkyboxMeshData->m_MeshVec[0]->IndexCount,
			m_pSkyboxMeshData->m_MeshVec[0]->StartIndex,
			0);
	}
}

void Skybox::Render()
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	unsigned int _stride = sizeof(Vertex::PosNormalTex);
	unsigned int _offset = 0;

	_dc->IASetVertexBuffers(0, 1, m_pSkyboxMeshData->m_VertexBuffer.GetAddressOf(), &_stride, &_offset);
	_dc->IASetIndexBuffer(m_pSkyboxMeshData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11ShaderResourceView* texture = m_pCubemap->GetSRV();
	Effects::SkyFX->SetCubeMap(texture);

	ID3DX11EffectTechnique* _activeTech = Effects::SkyFX->SkyTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	_activeTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		_activeTech->GetPassByIndex(p)->Apply(0, _dc);

		_dc->DrawIndexed(m_pSkyboxMeshData->m_MeshVec[0]->IndexCount,
			m_pSkyboxMeshData->m_MeshVec[0]->StartIndex,
			0);
	}

	//DebugManager::GetInstance()->AddSRV(m_Cubemap->GetSRV(), L"Cubemap");
	//DebugManager::GetInstance()->AddSRV(m_IrradianceMap->GetSRV(), L"IrradianceMap");
	//DebugManager::GetInstance()->AddSRV(m_PreFilterMap->GetSRV(), L"PreFilterMap");

	DebugManager::GetInstance()->AddRenderTarget(m_pBrdfLUT.get());
}
