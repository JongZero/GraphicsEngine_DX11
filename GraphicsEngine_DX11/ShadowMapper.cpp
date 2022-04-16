#include "ShadowMapper.h"
#include "DX11Core.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "ClientSize.h"
#include "MacroDefine.h"
#include "ErrChecker.h"
#include "ResourceManager.h"
#include "ObjectMeshData.h"
#include "Mesh.h"
#include "RenderTargetDrawer.h"
#include "ICore.h"
#include "MathConverter.h"
#include "ECollision.h"

ShadowMapper::ShadowMapper(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm, int width, int height)
	: m_DX11Core(dx11Core), m_ResourceManager(rm), m_Width(width), m_Height(height), IsActive(true)
{
	Initialize(width, height);
}

ShadowMapper::~ShadowMapper()
{
}

void ShadowMapper::Initialize(int width, int height)
{
	m_Width = width;
	m_Height = height;

	// 그림자 맵의 크기와 일치하는 뷰포트
	m_ViewPort.TopLeftX = 0.0f;
	m_ViewPort.TopLeftY = 0.0f;
	m_ViewPort.Width = static_cast<float>(width);
	m_ViewPort.Height = static_cast<float>(height);
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;

	// 깊이 스텐실 뷰는 비트들을 DXGI_FORMAT_D24_UNORM_S8_UINT로 해석하는 반면
	// 셰이더 자원 뷰는 비트들을 DXGI_FORMAT_R24_UNORM_X8_TYPELESS로 해석할
	// 것이므로 무형식 텍스처를 사용한다.
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Device* device = m_DX11Core->GetDevice();

	ID3D11Texture2D* depthMap = 0;
	HR(device->CreateTexture2D(&texDesc, 0, &depthMap));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(device->CreateDepthStencilView(depthMap, &dsvDesc, m_DepthStencilView.GetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	HR(device->CreateShaderResourceView(depthMap, &srvDesc, m_DepthMap.GetAddressOf()));

	// 텍스처에 대한 참조는 뷰에 저장되어 있으므로 여기서 참조를 해제해도 된다.
	ReleaseCOM(depthMap);
}

void ShadowMapper::StartMap(Shared_RenderingData* dataForRender, ICore* pCore)
{
	/// 그림자를 그려야하는 라이트를 찾는다.
	/// 지금은 일단 Main Directional Light에만 그림자를 적용시킨다.
	/// 그림자를 그려야하는 Light의 View, Proj을 구하고, Shadow Transform을 구한다.
	for (const auto& it : dataForRender->m_DirectionalLightInfoVec)
	{
		if (it->IsCastShadow)
		{
			BuildShadowTransform(it);
			break;
		}
	}

	PreMap();

	for (const auto& it : dataForRender->m_ObjectDataForRenderVec)
	{
		/// 쉐도우를 그릴지 말지 결정하고 그려야함 (그림자를 그릴지 말지는 게임 엔진으로부터 받아옴)
		if (it->m_IsCastShadow)
		{
			Map(it);
		}
	}

	PostMap(pCore);
}

void ShadowMapper::BuildShadowTransform(DirectionalLightInfo* mainDirInfo)
{
	/// 씬의 경계구를 게임엔진으로부터 받아와야한다. *_*

	// 장면의 구성을 미리 알고 있으므로, 수치들을 직접 지정해서 장면의 경계구를 추정한다.
	// 장면에서 너비가 가장 넓은 물체는 바닥의 격자로, 세계 공간의 원점에 놓여 있으며 
	// 너비가 20이고 깊이는 30.0f이다.
	// 일반적으로는 세계 공간의 모든 정점 위치를 훑어서 경계구의 반지름을 구해야할 것이다.
	ECollision::BoundingSphere _sceneBoundingSphere;

	_sceneBoundingSphere.Center = EMath::Vector3(0.0f, 0.0f, 0.0f);
	_sceneBoundingSphere.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f) * 2;

	// Build Shadow Transform
	// 첫 번째의 '주' 광원만 그림자를 드리운다.
	EMath::Vector3 dirLightdir = mainDirInfo->Direction;
	EMath::Vector3 lightDir = dirLightdir;

	EMath::Vector3 lightPos;
	EMath::Vector3 targetPos;
	
	lightPos = -2.0f * _sceneBoundingSphere.Radius * lightDir;
	targetPos = _sceneBoundingSphere.Center;

	EMath::Vector3 up(0, 1, 0);

	EMath::Matrix V = EMath::Matrix::CreateLookAt(lightPos, targetPos, up);

	// 경계구를 광원 공간으로 변환한다.
	EMath::Vector4 sphereCenterLS = EMath::Vector3::TransformCoord(targetPos, V);

	// 장면을 감싸는 광원 공간 직교투영 상자
	float l = sphereCenterLS.x - _sceneBoundingSphere.Radius;
	float b = sphereCenterLS.y - _sceneBoundingSphere.Radius;
	float n = sphereCenterLS.z - _sceneBoundingSphere.Radius;
	float r = sphereCenterLS.x + _sceneBoundingSphere.Radius;
	float t = sphereCenterLS.y + _sceneBoundingSphere.Radius;
	float f = sphereCenterLS.z + _sceneBoundingSphere.Radius;
	EMath::Matrix P = EMath::Matrix::CreateOrthographicOffCenter(l, r, b, t, n, f);

	// NDC 공간 [-1, +1]^2를 텍스처 공간 [0,1]^2으로 변환한다.
	EMath::Matrix T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	EMath::Matrix S = V * P * T;

	m_LightView = V;
	m_LightProj = P;
	m_ShadowTransform = S;
}

void ShadowMapper::PreMap()
{
	ID3D11DeviceContext* dc = m_DX11Core->GetDC();
	dc->RSSetViewports(1, &m_ViewPort);

	// 깊이 장면에만 장면을 그릴 것이므로 렌더 타겟을 NULL로 설정한다.
	// NULL 렌더 타겟을 설정하면 색상 쓰기가 비활성화된다.
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	dc->OMSetRenderTargets(1, renderTargets, m_DepthStencilView.Get());
	dc->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	dc->RSSetState(RenderStates::GetSolidRS());
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ShadowMapper::Map(const Shared_ObjectData* objectDataForRender)
{
	ID3D11DeviceContext* dc = m_DX11Core->GetDC();

	// 리소스 매니저로부터 오브젝트의 메쉬를 가져온다.
	ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(objectDataForRender->m_Name);

	// 메쉬가 없거나 헬퍼 오브젝트는 그림자를 그리지 않음
	if (_objMeshData == nullptr 
		|| _objMeshData->m_ObjectMeshInfo.m_EffectType == ObjectMeshInfo::eEffectType::Color)
		return;

	// Input Layout
	ID3D11InputLayout* inputLayout = nullptr;

	switch (_objMeshData->m_ObjectMeshInfo.m_EffectType)
	{
	case ObjectMeshInfo::eEffectType::Basic:
		inputLayout = InputLayouts::PosNormalTex;
		break;
	case ObjectMeshInfo::eEffectType::NormalMap:
		inputLayout = InputLayouts::PosNormalTexTangentU;
		break;
	case ObjectMeshInfo::eEffectType::Skinning:
		inputLayout = InputLayouts::PosNormalTexTangentUWeightsBoneIndices;
		break;
	}

	assert(inputLayout);
	dc->IASetInputLayout(inputLayout);

	// VB, IB
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, _objMeshData->m_VertexBuffer.GetAddressOf(), &_objMeshData->m_Stride, &offset);
	dc->IASetIndexBuffer(_objMeshData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Tech
	ID3DX11EffectTechnique* tech = nullptr;
	switch (_objMeshData->m_ObjectMeshInfo.m_EffectType)
	{
	case ObjectMeshInfo::eEffectType::Basic:
		tech = Effects::ShadowMapFX->BuildShadowMap_BasicTech;
		break;
	case ObjectMeshInfo::eEffectType::NormalMap:
		tech = Effects::ShadowMapFX->BuildShadowMap_NormalMapTech;
		break;
	case ObjectMeshInfo::eEffectType::Skinning:
		tech = Effects::ShadowMapFX->BuildShadowMap_SkinningTech;
		break;
	}

	assert(tech);
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	// WorldTM
	EMath::Matrix world = objectDataForRender->m_World;

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		for (UINT m = 0; m < _objMeshData->m_MeshVec.size(); m++)
		{
			if (_objMeshData->m_MeshVec[m]->IsBone)
				continue;

			/// Mesh Renderer와 마찬가지로 해야한다.
			EMath::Matrix meshWorld = _objMeshData->m_MeshVec[m]->WorldTM;
			meshWorld *= _objMeshData->m_MeshVec[m]->NodeTM;

			meshWorld = meshWorld * world;
			EMath::Matrix worldViewProj = meshWorld * m_LightView * m_LightProj;

			Effects::ShadowMapFX->SetWorldViewProj(worldViewProj);
			//Effects::ShadowMapFX->SetTexTransform(XMMatrixScaling(2.0f, 1.0f, 1.0f));
			Effects::ShadowMapFX->SetTexTransform(EMath::Matrix::Identity);

			tech->GetPassByIndex(p)->Apply(0, dc);
			dc->DrawIndexed(_objMeshData->m_MeshVec[m]->IndexCount,
				_objMeshData->m_MeshVec[m]->StartIndex,
				0);
		}
	}
}

void ShadowMapper::PostMap(ICore* pCore)
{
	// 매핑이 끝났으므로 쉐이더에 맵을 보냄
	Effects::SetShadowMap(m_DepthMap.Get());

	// 뷰포트를 다시 원상복귀한다.
	pCore->SetMainViewport();
}

void ShadowMapper::DrawDebugScreen()
{
	/// <summary>
	/// 추후에 쉐도우 맵이 2개 이상 늘어날 경우에는
	/// 월드 TM을 개별로 받아야하기 때문에 생성자에서 받아줘야한다.
	/// </summary>
	EMath::Matrix world(
		0.245f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.245f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.245f, 0.0f,
		0.75f, 0.75f, 0.0f, 1.0f);

	RenderTargetDrawer::DrawRenderTarget(m_DX11Core->GetDC(), m_DepthMap.Get(), world, Effects::DebugTexFX->ViewRedTech);
}
