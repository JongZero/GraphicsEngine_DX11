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

	// 라플라시안 필터를 사용하는지, 오버 사이즈 타입을 사용하는 지에 따라 분기가 나뉜다.
	// (라플라시안 필터를 사용할 경우엔 노멀 벡터를 필요로하고 
	// 오버 사이즈의 경우에는 오브젝트를 키우기 전의 뎁스가 필요하고(스텐실 마스킹을 위해), 오브젝트를 두께만큼 더 크게 그려야한다.)
	switch (m_Type)
	{
	case OutlineDrawer::Type::LaplacianFilter:
		// 노말을 기록
		DrawNormal(dataForRender);

		// 기록된 노말을 가지고 라플라시안 필터를 이용해 외곽선 검출
		DrawOutlineByLaplacianFilter(output);
		break;
	case OutlineDrawer::Type::OverSize:
		// 외곽선의 두께
		m_Size = size;

		// 오버 사이즈하기 전의 뎁스를 기록
		DrawDepth(dataForRender);

		// 오버 사이즈로 오브젝트를 그림
		DrawOutlineByOverSize(output, dataForRender);
		break;
	}
}

void OutlineDrawer::Draw(RenderTarget* output, RenderTarget* lastRenderTarget, RenderTarget* preOutlineOutput)
{
	// 그려놓은 외곽선을 Final 렌더타겟과 합친다.
	// 블러한 경우 블러한 것과 합친다.
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
	depthStencilDesc.Width = width;			// 텍스처의 너비(텍셀 단위)
	depthStencilDesc.Height = height;			// 텍스처의 높이(텍셀 단위)
	depthStencilDesc.MipLevels = 1;								// 밉맵 수준의 갯수, 깊이·스텐실 버퍼를 위한 텍스처에서는 밉맵 수준이 하나만 있으면 된다.
	depthStencilDesc.ArraySize = 1;								// 텍스처 배열의 텍스처 갯수, 깊이·스텐실 버퍼의 경우에는 텍스처 하나만 필요하다.
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;	// 텍셀의 형식, 깊이·스텐실 버퍼의 경우 용책 p109의 형식 중 하나로 지정해야한다. (자세한 내용 p109 참고)

	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;			// 텍스처의 용도, D3D11_USAGE_DEFAULT : 자원을 GPU가 읽고 써야 한다면 이 용도를 설정한다. 이 용도를 설정하면 CPU는 자원을 읽거나 쓸 수 없다. 
															// 깊이·스텐실 버퍼에 대한 모든 읽기, 쓰기는 GPU가 수행하므로 깊이·스텐실 버퍼를 위한 텍스처를 생성할 때에는 바로 이 D3D11_USAGE_DEFAULT를 사용한다.
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;	// 자원을 파이프라인에 어떤 식으로 묶을 것인지를 지정하는 하나 이상의 플래그들을 OR로 결합해서 지정한다. 깊이·스텐실 버퍼의 경우 D3D11_BIND_DEPTH_STENCIL 플래그를 지정해야 한다.
	depthStencilDesc.CPUAccessFlags = 0;					// CPU가 자원에 접근하는 방식을 결정하는 플래그들을 지정한다. 깊이·스텐실 버퍼의 경우 GPU만 깊이·스텐실 버퍼를 읽고 쓸 뿐 CPU는 전혀 접근하지 않으므로 0을 지정
	depthStencilDesc.MiscFlags = 0;							// 기타 플래그들, 깊이·스텐실 버퍼에는 적용되지 않으므로 그냥 0 지정

	// DepthMap 기록을 위한 뎁스버퍼
	ID3D11Device* _device = m_DX11Core->GetDevice();
	HR(_device->CreateTexture2D(&depthStencilDesc,
		0,					// 텍스처에 채울 초기 자료를 가리키는 포인터, 이 텍스처는 깊이·스텐실 버퍼로 사용할 것이므로 따로 자료를 채울 필요 X
		m_DepthStencilBuffer_ForDepthMap.GetAddressOf()));
	
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	HR(_device->CreateDepthStencilView(m_DepthStencilBuffer_ForDepthMap.Get(), &dsvDesc, m_DepthStencilView_ForDepthMap.GetAddressOf()));

	// 일반 뎁스 버퍼
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	HR(_device->CreateTexture2D(&depthStencilDesc,
		0,					// 텍스처에 채울 초기 자료를 가리키는 포인터, 이 텍스처는 깊이·스텐실 버퍼로 사용할 것이므로 따로 자료를 채울 필요 X
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
		// 아웃라인을 그릴지 검사
		if (0 != wcscmp(it->m_Name, dataForRender->m_PickedObject))
		{
			continue;
		}

		// 오브젝트의 메쉬 데이터를 ID를 통해 리소스 매니저로부터 가져온다.
		ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(it->m_Name);

		if (_objMeshData == nullptr)
		{
			continue;
		}

		UINT offset = 0;
		m_DC->IASetVertexBuffers(0, 1, _objMeshData->m_VertexBuffer.GetAddressOf(), &_objMeshData->m_Stride, &offset);
		m_DC->IASetIndexBuffer(_objMeshData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		/// InputLayout 설정
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

		// 본이 있는 메쉬이고, 본 트랜스폼이 제대로 들어있으면
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
	// 외곽선을 그릴 오브젝트의 뎁스를 기록한다.
	// 깊이 장면에만 장면을 그릴 것이므로 렌더 타겟을 NULL로 설정한다.
	// NULL 렌더 타겟을 설정하면 색상 쓰기가 비활성화된다.
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	m_DC->OMSetRenderTargets(1, renderTargets, m_DepthStencilView_ForDepthMap.Get());
	m_DC->ClearDepthStencilView(m_DepthStencilView_ForDepthMap.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	for (const auto& it : dataForRender->m_ObjectDataForRenderVec)
	{
		// 아웃라인을 그릴지 검사
		if (0 != wcscmp(it->m_Name, dataForRender->m_PickedObject))
		{
			continue;
		}
		
		// 오브젝트의 메쉬 데이터를 ID를 통해 리소스 매니저로부터 가져온다.
		ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(it->m_Name);

		if (_objMeshData == nullptr)
		{
			continue;
		}

		/// InputLayout 설정
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

		// 본이 있는 메쉬이고, 본 트랜스폼이 제대로 들어있으면
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
		// 아웃라인을 그릴지 검사
		if (0 != wcscmp(it->m_Name, dataForRender->m_PickedObject))
		{
			continue;
		}

		// 오브젝트의 메쉬 데이터를 ID를 통해 리소스 매니저로부터 가져온다.
		ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(it->m_Name);

		if (_objMeshData == nullptr)
		{
			continue;
		}

		/// InputLayout 설정
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

		// 본이 있는 메쉬이고, 본 트랜스폼이 제대로 들어있으면
		if (_objMeshData->m_BoneFinalTransformVec.size() > 0)
		{
			Effects::OutlineFX->SetBoneTransforms(_objMeshData->m_BoneFinalTransformVec);
		}

		// 사이즈(두께 설정)
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
	// 이전에 저장해놓은 노말로 외곽선을 그린다.
	m_DC->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	m_DC->OMSetRenderTargets(1, &rt[0], m_DX11Core->GetDSV());

	m_DC->RSSetState(RenderStates::GetSolidRS());

	Effects::OutlineFX->SetNormalW(m_NormalW->GetSRV());
	Effects::OutlineFX->SetWorldViewProj(EMath::Matrix::Identity);

	ID3DX11EffectTechnique* tech = Effects::OutlineFX->Outline_LaplacianTech;
	RenderTargetDrawer::DrawRenderTarget(m_DC, tech);
}