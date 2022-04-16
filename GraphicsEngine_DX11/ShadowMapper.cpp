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

	// �׸��� ���� ũ��� ��ġ�ϴ� ����Ʈ
	m_ViewPort.TopLeftX = 0.0f;
	m_ViewPort.TopLeftY = 0.0f;
	m_ViewPort.Width = static_cast<float>(width);
	m_ViewPort.Height = static_cast<float>(height);
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;

	// ���� ���ٽ� ��� ��Ʈ���� DXGI_FORMAT_D24_UNORM_S8_UINT�� �ؼ��ϴ� �ݸ�
	// ���̴� �ڿ� ��� ��Ʈ���� DXGI_FORMAT_R24_UNORM_X8_TYPELESS�� �ؼ���
	// ���̹Ƿ� ������ �ؽ�ó�� ����Ѵ�.
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

	// �ؽ�ó�� ���� ������ �信 ����Ǿ� �����Ƿ� ���⼭ ������ �����ص� �ȴ�.
	ReleaseCOM(depthMap);
}

void ShadowMapper::StartMap(Shared_RenderingData* dataForRender, ICore* pCore)
{
	/// �׸��ڸ� �׷����ϴ� ����Ʈ�� ã�´�.
	/// ������ �ϴ� Main Directional Light���� �׸��ڸ� �����Ų��.
	/// �׸��ڸ� �׷����ϴ� Light�� View, Proj�� ���ϰ�, Shadow Transform�� ���Ѵ�.
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
		/// �����츦 �׸��� ���� �����ϰ� �׷����� (�׸��ڸ� �׸��� ������ ���� �������κ��� �޾ƿ�)
		if (it->m_IsCastShadow)
		{
			Map(it);
		}
	}

	PostMap(pCore);
}

void ShadowMapper::BuildShadowTransform(DirectionalLightInfo* mainDirInfo)
{
	/// ���� ��豸�� ���ӿ������κ��� �޾ƿ;��Ѵ�. *_*

	// ����� ������ �̸� �˰� �����Ƿ�, ��ġ���� ���� �����ؼ� ����� ��豸�� �����Ѵ�.
	// ��鿡�� �ʺ� ���� ���� ��ü�� �ٴ��� ���ڷ�, ���� ������ ������ ���� ������ 
	// �ʺ� 20�̰� ���̴� 30.0f�̴�.
	// �Ϲ������δ� ���� ������ ��� ���� ��ġ�� �Ⱦ ��豸�� �������� ���ؾ��� ���̴�.
	ECollision::BoundingSphere _sceneBoundingSphere;

	_sceneBoundingSphere.Center = EMath::Vector3(0.0f, 0.0f, 0.0f);
	_sceneBoundingSphere.Radius = sqrtf(10.0f * 10.0f + 15.0f * 15.0f) * 2;

	// Build Shadow Transform
	// ù ��°�� '��' ������ �׸��ڸ� �帮���.
	EMath::Vector3 dirLightdir = mainDirInfo->Direction;
	EMath::Vector3 lightDir = dirLightdir;

	EMath::Vector3 lightPos;
	EMath::Vector3 targetPos;
	
	lightPos = -2.0f * _sceneBoundingSphere.Radius * lightDir;
	targetPos = _sceneBoundingSphere.Center;

	EMath::Vector3 up(0, 1, 0);

	EMath::Matrix V = EMath::Matrix::CreateLookAt(lightPos, targetPos, up);

	// ��豸�� ���� �������� ��ȯ�Ѵ�.
	EMath::Vector4 sphereCenterLS = EMath::Vector3::TransformCoord(targetPos, V);

	// ����� ���δ� ���� ���� �������� ����
	float l = sphereCenterLS.x - _sceneBoundingSphere.Radius;
	float b = sphereCenterLS.y - _sceneBoundingSphere.Radius;
	float n = sphereCenterLS.z - _sceneBoundingSphere.Radius;
	float r = sphereCenterLS.x + _sceneBoundingSphere.Radius;
	float t = sphereCenterLS.y + _sceneBoundingSphere.Radius;
	float f = sphereCenterLS.z + _sceneBoundingSphere.Radius;
	EMath::Matrix P = EMath::Matrix::CreateOrthographicOffCenter(l, r, b, t, n, f);

	// NDC ���� [-1, +1]^2�� �ؽ�ó ���� [0,1]^2���� ��ȯ�Ѵ�.
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

	// ���� ��鿡�� ����� �׸� ���̹Ƿ� ���� Ÿ���� NULL�� �����Ѵ�.
	// NULL ���� Ÿ���� �����ϸ� ���� ���Ⱑ ��Ȱ��ȭ�ȴ�.
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	dc->OMSetRenderTargets(1, renderTargets, m_DepthStencilView.Get());
	dc->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	dc->RSSetState(RenderStates::GetSolidRS());
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ShadowMapper::Map(const Shared_ObjectData* objectDataForRender)
{
	ID3D11DeviceContext* dc = m_DX11Core->GetDC();

	// ���ҽ� �Ŵ����κ��� ������Ʈ�� �޽��� �����´�.
	ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(objectDataForRender->m_Name);

	// �޽��� ���ų� ���� ������Ʈ�� �׸��ڸ� �׸��� ����
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

			/// Mesh Renderer�� ���������� �ؾ��Ѵ�.
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
	// ������ �������Ƿ� ���̴��� ���� ����
	Effects::SetShadowMap(m_DepthMap.Get());

	// ����Ʈ�� �ٽ� ���󺹱��Ѵ�.
	pCore->SetMainViewport();
}

void ShadowMapper::DrawDebugScreen()
{
	/// <summary>
	/// ���Ŀ� ������ ���� 2�� �̻� �þ ��쿡��
	/// ���� TM�� ������ �޾ƾ��ϱ� ������ �����ڿ��� �޾�����Ѵ�.
	/// </summary>
	EMath::Matrix world(
		0.245f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.245f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.245f, 0.0f,
		0.75f, 0.75f, 0.0f, 1.0f);

	RenderTargetDrawer::DrawRenderTarget(m_DX11Core->GetDC(), m_DepthMap.Get(), world, Effects::DebugTexFX->ViewRedTech);
}
