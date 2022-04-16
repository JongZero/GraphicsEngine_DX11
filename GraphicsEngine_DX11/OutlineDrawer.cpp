#include "OutlineDrawer.h"
#include "RenderTargetDrawer.h"
#include "RenderTarget.h"
#include "ResourceManager.h"
#include "Shared_RenderingData.h"
#include "DX11Core.h"
#include "RenderStates.h"
#include "ObjectMeshData.h"
#include "Vertex.h"
#include "Mesh.h"
#include "Mathhelper.h"
#include "ErrChecker.h"
#include "EColors.h"

#include "ClientSize.h"
#include <assert.h>

OutlineDrawer::OutlineDrawer(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm)
	: m_DX11Core(dx11Core), m_ResourceManager(rm), m_Size(1.5f)
{
	int width = ClientSize::GetWidth();
	int height = ClientSize::GetHeight();

	CreateRenderTargets();
	OnResizeDepthBuffers(width, height);
}

OutlineDrawer::~OutlineDrawer()
{
	
}

void OutlineDrawer::PrePass(RenderTarget* output, Shared_RenderingData* dataForRender, Type type, float size /*= 1.5f*/)
{
	m_DC = m_DX11Core->GetDC();
	m_Type = type;

	m_DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_DC->RSSetState(RenderStates::GetSolidRS());

	// ���ö�þ� ���͸� ����ϴ���, ���� ������ Ÿ���� ����ϴ� ���� ���� �бⰡ ������.
	// (���ö�þ� ���͸� ����� ��쿣 ��� ���͸� �ʿ���ϰ� 
	// ���� �������� ��쿡�� ������Ʈ�� Ű��� ���� ������ �ʿ��ϰ�(���ٽ� ����ŷ�� ����), ������Ʈ�� �β���ŭ �� ũ�� �׷����Ѵ�.)
	switch (m_Type)
	{
	case OutlineDrawer::Type::LaplacianFilter:
		// �븻�� ���
		DrawNormal(dataForRender);

		// ��ϵ� �븻�� ������ ���ö�þ� ���͸� �̿��� �ܰ��� ����
		DrawOutlineByLaplacianFilter(output);
		break;
	case OutlineDrawer::Type::OverSize:
		// �ܰ����� �β�
		m_Size = size;

		// ���� �������ϱ� ���� ������ ���
		DrawDepth(dataForRender);

		// ���� ������� ������Ʈ�� �׸�
		DrawOutlineByOverSize(output, dataForRender);
		break;
	}
}

void OutlineDrawer::Draw(RenderTarget* output, RenderTarget* lastRenderTarget, RenderTarget* preOutlineOutput)
{
	// �׷����� �ܰ����� Final ����Ÿ�ٰ� ��ģ��.
	// ���� ��� ���� �Ͱ� ��ģ��.
	m_DC->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	ID3D11RenderTargetView* rt2[] = { output->GetRTV() };
	m_DC->OMSetRenderTargets(1, &rt2[0], m_DX11Core->GetDSV());

	Effects::OutlineFX->SetFinal(lastRenderTarget->GetSRV());

	ID3DX11EffectTechnique* tech = nullptr;

	switch (m_Type)
	{
	case OutlineDrawer::Type::LaplacianFilter:
		tech = Effects::OutlineFX->Mix_LaplacianTech;
		break;
	case OutlineDrawer::Type::OverSize:
		tech = Effects::OutlineFX->Mix_OverSizeTech;
		Effects::OutlineFX->SetDepthMap(m_DepthMap.Get());
		break;
	}
	
	assert(tech);

	Effects::OutlineFX->SetOutline(preOutlineOutput->GetSRV());
	Effects::OutlineFX->SetWorldViewProj(EMath::Matrix::Identity);

	RenderTargetDrawer::DrawRenderTarget(m_DC, tech);
}

void OutlineDrawer::OnResize(int width, int height)
{
	m_NormalW->OnResize(m_DX11Core->GetDevice(), width, height);

	m_DepthStencilBuffer_ForDepthMap = nullptr;
	m_DepthStencilView_ForDepthMap = nullptr;
	m_DepthMap = nullptr;

	m_DepthStencilBuffer = nullptr;
	m_DepthStencilView = nullptr;

	OnResizeDepthBuffers(width, height);
}

void OutlineDrawer::CreateRenderTargets()
{
	int width = ClientSize::GetWidth();
	int height = ClientSize::GetHeight();

	m_NormalW = std::make_unique<RenderTarget>(width, height, L"< Normal For Outline >");
}

void OutlineDrawer::OnResizeDepthBuffers(int width, int height)
{
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = width;			// �ؽ�ó�� �ʺ�(�ؼ� ����)
	depthStencilDesc.Height = height;			// �ؽ�ó�� ����(�ؼ� ����)
	depthStencilDesc.MipLevels = 1;								// �Ӹ� ������ ����, ���̡����ٽ� ���۸� ���� �ؽ�ó������ �Ӹ� ������ �ϳ��� ������ �ȴ�.
	depthStencilDesc.ArraySize = 1;								// �ؽ�ó �迭�� �ؽ�ó ����, ���̡����ٽ� ������ ��쿡�� �ؽ�ó �ϳ��� �ʿ��ϴ�.
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;	// �ؼ��� ����, ���̡����ٽ� ������ ��� ��å p109�� ���� �� �ϳ��� �����ؾ��Ѵ�. (�ڼ��� ���� p109 ����)

	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;			// �ؽ�ó�� �뵵, D3D11_USAGE_DEFAULT : �ڿ��� GPU�� �а� ��� �Ѵٸ� �� �뵵�� �����Ѵ�. �� �뵵�� �����ϸ� CPU�� �ڿ��� �аų� �� �� ����. 
															// ���̡����ٽ� ���ۿ� ���� ��� �б�, ����� GPU�� �����ϹǷ� ���̡����ٽ� ���۸� ���� �ؽ�ó�� ������ ������ �ٷ� �� D3D11_USAGE_DEFAULT�� ����Ѵ�.
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;	// �ڿ��� ���������ο� � ������ ���� �������� �����ϴ� �ϳ� �̻��� �÷��׵��� OR�� �����ؼ� �����Ѵ�. ���̡����ٽ� ������ ��� D3D11_BIND_DEPTH_STENCIL �÷��׸� �����ؾ� �Ѵ�.
	depthStencilDesc.CPUAccessFlags = 0;					// CPU�� �ڿ��� �����ϴ� ����� �����ϴ� �÷��׵��� �����Ѵ�. ���̡����ٽ� ������ ��� GPU�� ���̡����ٽ� ���۸� �а� �� �� CPU�� ���� �������� �����Ƿ� 0�� ����
	depthStencilDesc.MiscFlags = 0;							// ��Ÿ �÷��׵�, ���̡����ٽ� ���ۿ��� ������� �����Ƿ� �׳� 0 ����

	// DepthMap ����� ���� ��������
	ID3D11Device* _device = m_DX11Core->GetDevice();
	HR(_device->CreateTexture2D(&depthStencilDesc,
		0,					// �ؽ�ó�� ä�� �ʱ� �ڷḦ ����Ű�� ������, �� �ؽ�ó�� ���̡����ٽ� ���۷� ����� ���̹Ƿ� ���� �ڷḦ ä�� �ʿ� X
		m_DepthStencilBuffer_ForDepthMap.GetAddressOf()));
	
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(_device->CreateDepthStencilView(m_DepthStencilBuffer_ForDepthMap.Get(), &dsvDesc, m_DepthStencilView_ForDepthMap.GetAddressOf()));

	// �Ϲ� ���� ����
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	HR(_device->CreateTexture2D(&depthStencilDesc,
		0,					// �ؽ�ó�� ä�� �ʱ� �ڷḦ ����Ű�� ������, �� �ؽ�ó�� ���̡����ٽ� ���۷� ����� ���̹Ƿ� ���� �ڷḦ ä�� �ʿ� X
		m_DepthStencilBuffer.GetAddressOf()));
	HR(_device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), 0, m_DepthStencilView.GetAddressOf()));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = depthStencilDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	HR(_device->CreateShaderResourceView(m_DepthStencilBuffer_ForDepthMap.Get(), &srvDesc, m_DepthMap.GetAddressOf()));
}

void OutlineDrawer::DrawNormal(Shared_RenderingData* dataForRender)
{
	m_DC->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_DC->ClearRenderTargetView(m_NormalW->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	ID3D11RenderTargetView* rt[] = { m_NormalW->GetRTV() };
	m_DC->OMSetRenderTargets(1, &rt[0], m_DepthStencilView.Get());

	for (const auto& it : dataForRender->m_ObjectDataForRenderVec)
	{
		// �ƿ������� �׸��� �˻�
		if (0 != wcscmp(it->m_Name, dataForRender->m_PickedObject))
		{
			continue;
		}

		// ������Ʈ�� �޽� �����͸� ID�� ���� ���ҽ� �Ŵ����κ��� �����´�.
		ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(it->m_Name);

		if (_objMeshData == nullptr)
		{
			continue;
		}

		UINT offset = 0;
		m_DC->IASetVertexBuffers(0, 1, _objMeshData->m_VertexBuffer.GetAddressOf(), &_objMeshData->m_Stride, &offset);
		m_DC->IASetIndexBuffer(_objMeshData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		/// InputLayout ����
		ID3D11InputLayout* _inputLayout = nullptr;

		switch (_objMeshData->m_ObjectMeshInfo.m_EffectType)
		{
		case ObjectMeshInfo::eEffectType::Basic:
			_inputLayout = InputLayouts::PosNormalTex;
			break;
		case ObjectMeshInfo::eEffectType::NormalMap:
			_inputLayout = InputLayouts::PosNormalTexTangentU;
			break;
		case ObjectMeshInfo::eEffectType::Skinning:
			_inputLayout = InputLayouts::PosNormalTexTangentUWeightsBoneIndices;
			break;
		}

		assert(_inputLayout);
		m_DC->IASetInputLayout(_inputLayout);

		/// Set Active Tech
		ID3DX11EffectTechnique* _activeTech = nullptr;

		switch (_objMeshData->m_ObjectMeshInfo.m_EffectType)
		{
		case ObjectMeshInfo::eEffectType::Basic:
			_activeTech = Effects::OutlineFX->Basic_NormalTech;
			break;
		case ObjectMeshInfo::eEffectType::NormalMap:
			_activeTech = Effects::OutlineFX->NormalMap_NormalTech;
			break;
		case ObjectMeshInfo::eEffectType::Skinning:
			_activeTech = Effects::OutlineFX->Skinning_NormalTech;
			break;
		}

		// ���� �ִ� �޽��̰�, �� Ʈ�������� ����� ���������
		if (_objMeshData->m_BoneFinalTransformVec.size() > 0)
		{
			Effects::OutlineFX->SetBoneTransforms(_objMeshData->m_BoneFinalTransformVec);
		}

		D3DX11_TECHNIQUE_DESC techDesc;
		_activeTech->GetDesc(&techDesc);

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			for (UINT m = 0; m < _objMeshData->m_MeshVec.size(); m++)
			{
				if (_objMeshData->m_MeshVec[m]->IsBone)
					continue;

				EMath::Matrix meshWorld = _objMeshData->m_MeshVec[m]->WorldTM;

				meshWorld = meshWorld * it->m_World;

				EMath::Matrix worldViewProj = meshWorld * dataForRender->m_View * dataForRender->m_Proj;
				EMath::Matrix worldInvTranspose = MathHelper::InverseTranspose(meshWorld);

				Effects::OutlineFX->SetWorldViewProj(worldViewProj);
				Effects::OutlineFX->SetWorldInvTranspose(worldInvTranspose);

				_activeTech->GetPassByIndex(p)->Apply(0, m_DC);
				m_DC->DrawIndexed(_objMeshData->m_MeshVec[m]->IndexCount,
					_objMeshData->m_MeshVec[m]->StartIndex,
					0);
			}
		}
	}
}

void OutlineDrawer::DrawDepth(Shared_RenderingData* dataForRender)
{
	// �ܰ����� �׸� ������Ʈ�� ������ ����Ѵ�.
	// ���� ��鿡�� ����� �׸� ���̹Ƿ� ���� Ÿ���� NULL�� �����Ѵ�.
	// NULL ���� Ÿ���� �����ϸ� ���� ���Ⱑ ��Ȱ��ȭ�ȴ�.
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	m_DC->OMSetRenderTargets(1, renderTargets, m_DepthStencilView_ForDepthMap.Get());
	m_DC->ClearDepthStencilView(m_DepthStencilView_ForDepthMap.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	for (const auto& it : dataForRender->m_ObjectDataForRenderVec)
	{
		// �ƿ������� �׸��� �˻�
		if (0 != wcscmp(it->m_Name, dataForRender->m_PickedObject))
		{
			continue;
		}
		
		// ������Ʈ�� �޽� �����͸� ID�� ���� ���ҽ� �Ŵ����κ��� �����´�.
		ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(it->m_Name);

		if (_objMeshData == nullptr)
		{
			continue;
		}

		/// InputLayout ����
		ID3D11InputLayout* _inputLayout = nullptr;

		switch (_objMeshData->m_ObjectMeshInfo.m_EffectType)
		{
		case ObjectMeshInfo::eEffectType::Basic:
			_inputLayout = InputLayouts::PosNormalTex;
			break;
		case ObjectMeshInfo::eEffectType::NormalMap:
			_inputLayout = InputLayouts::PosNormalTexTangentU;
			break;
		case ObjectMeshInfo::eEffectType::Skinning:
			_inputLayout = InputLayouts::PosNormalTexTangentUWeightsBoneIndices;
			break;
		}

		assert(_inputLayout);
		m_DC->IASetInputLayout(_inputLayout);

		UINT offset = 0;
		m_DC->IASetVertexBuffers(0, 1, _objMeshData->m_VertexBuffer.GetAddressOf(), &_objMeshData->m_Stride, &offset);
		m_DC->IASetIndexBuffer(_objMeshData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		/// Set Active Tech
		ID3DX11EffectTechnique* _activeTech = nullptr;

		switch (_objMeshData->m_ObjectMeshInfo.m_EffectType)
		{
		case ObjectMeshInfo::eEffectType::Basic:
			_activeTech = Effects::OutlineFX->Basic_DepthTech;
			break;
		case ObjectMeshInfo::eEffectType::NormalMap:
			_activeTech = Effects::OutlineFX->NormalMap_DepthTech;
			break;
		case ObjectMeshInfo::eEffectType::Skinning:
			_activeTech = Effects::OutlineFX->Skinning_DepthTech;
			break;
		}

		// ���� �ִ� �޽��̰�, �� Ʈ�������� ����� ���������
		if (_objMeshData->m_BoneFinalTransformVec.size() > 0)
		{
			Effects::OutlineFX->SetBoneTransforms(_objMeshData->m_BoneFinalTransformVec);
		}

		D3DX11_TECHNIQUE_DESC techDesc;
		_activeTech->GetDesc(&techDesc);

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			for (UINT m = 0; m < _objMeshData->m_MeshVec.size(); m++)
			{
				if (_objMeshData->m_MeshVec[m]->IsBone)
					continue;

				EMath::Matrix meshWorld = _objMeshData->m_MeshVec[m]->WorldTM;

				meshWorld = meshWorld * it->m_World;

				EMath::Matrix worldViewProj = meshWorld * dataForRender->m_View * dataForRender->m_Proj;
				EMath::Matrix worldInvTranspose = MathHelper::InverseTranspose(meshWorld);

				Effects::OutlineFX->SetWorldViewProj(worldViewProj);
				Effects::OutlineFX->SetWorldInvTranspose(worldInvTranspose);

				_activeTech->GetPassByIndex(p)->Apply(0, m_DC);
				m_DC->DrawIndexed(_objMeshData->m_MeshVec[m]->IndexCount,
					_objMeshData->m_MeshVec[m]->StartIndex,
					0);
			}
		}
	}
}

void OutlineDrawer::DrawOutlineByOverSize(RenderTarget* output, Shared_RenderingData* dataForRender)
{
	m_DC->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_DC->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	m_DC->OMSetRenderTargets(1, &rt[0], m_DepthStencilView.Get());
	//m_DC->OMSetRenderTargets(1, &rt[0], DXGetter::GetDSV());

	for (const auto& it : dataForRender->m_ObjectDataForRenderVec)
	{
		// �ƿ������� �׸��� �˻�
		if (0 != wcscmp(it->m_Name, dataForRender->m_PickedObject))
		{
			continue;
		}

		// ������Ʈ�� �޽� �����͸� ID�� ���� ���ҽ� �Ŵ����κ��� �����´�.
		ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(it->m_Name);

		if (_objMeshData == nullptr)
		{
			continue;
		}

		/// InputLayout ����
		ID3D11InputLayout* _inputLayout = nullptr;

		switch (_objMeshData->m_ObjectMeshInfo.m_EffectType)
		{
		case ObjectMeshInfo::eEffectType::Basic:
			_inputLayout = InputLayouts::PosNormalTex;
			break;
		case ObjectMeshInfo::eEffectType::NormalMap:
			_inputLayout = InputLayouts::PosNormalTexTangentU;
			break;
		case ObjectMeshInfo::eEffectType::Skinning:
			_inputLayout = InputLayouts::PosNormalTexTangentUWeightsBoneIndices;
			break;
		}

		assert(_inputLayout);
		m_DC->IASetInputLayout(_inputLayout);

		UINT offset = 0;
		m_DC->IASetVertexBuffers(0, 1, _objMeshData->m_VertexBuffer.GetAddressOf(), &_objMeshData->m_Stride, &offset);
		m_DC->IASetIndexBuffer(_objMeshData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		/// Set Active Tech
		ID3DX11EffectTechnique* _activeTech = nullptr;

		switch (_objMeshData->m_ObjectMeshInfo.m_EffectType)
		{
		case ObjectMeshInfo::eEffectType::Basic:
			_activeTech = Effects::OutlineFX->Basic_OverSizeTech;
			break;
		case ObjectMeshInfo::eEffectType::NormalMap:
			_activeTech = Effects::OutlineFX->NormalMap_OverSizeTech;
			break;
		case ObjectMeshInfo::eEffectType::Skinning:
			_activeTech = Effects::OutlineFX->Skinning_OverSizeTech;
			break;
		}

		// ���� �ִ� �޽��̰�, �� Ʈ�������� ����� ���������
		if (_objMeshData->m_BoneFinalTransformVec.size() > 0)
		{
			Effects::OutlineFX->SetBoneTransforms(_objMeshData->m_BoneFinalTransformVec);
		}

		// ������(�β� ����)
		Effects::OutlineFX->SetSize(m_Size);

		D3DX11_TECHNIQUE_DESC techDesc;
		_activeTech->GetDesc(&techDesc);

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			for (UINT m = 0; m < _objMeshData->m_MeshVec.size(); m++)
			{
				if (_objMeshData->m_MeshVec[m]->IsBone)
					continue;

				EMath::Matrix meshWorld = _objMeshData->m_MeshVec[m]->WorldTM;

				meshWorld = meshWorld * it->m_World;

				EMath::Matrix worldViewProj = meshWorld * dataForRender->m_View * dataForRender->m_Proj;
				EMath::Matrix worldInvTranspose = MathHelper::InverseTranspose(meshWorld);

				Effects::OutlineFX->SetWorldViewProj(worldViewProj);
				Effects::OutlineFX->SetWorldInvTranspose(worldInvTranspose);

				_activeTech->GetPassByIndex(p)->Apply(0, m_DC);
				m_DC->DrawIndexed(_objMeshData->m_MeshVec[m]->IndexCount,
					_objMeshData->m_MeshVec[m]->StartIndex,
					0);
			}
		}
	}
}

void OutlineDrawer::DrawOutlineByLaplacianFilter(RenderTarget* output)
{
	// ������ �����س��� �븻�� �ܰ����� �׸���.
	m_DC->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	m_DC->OMSetRenderTargets(1, &rt[0], m_DX11Core->GetDSV());

	m_DC->RSSetState(RenderStates::GetSolidRS());

	Effects::OutlineFX->SetNormalW(m_NormalW->GetSRV());
	Effects::OutlineFX->SetWorldViewProj(EMath::Matrix::Identity);

	ID3DX11EffectTechnique* tech = Effects::OutlineFX->Outline_LaplacianTech;
	RenderTargetDrawer::DrawRenderTarget(m_DC, tech);
}