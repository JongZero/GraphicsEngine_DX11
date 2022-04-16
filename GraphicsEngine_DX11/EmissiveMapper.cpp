#include "EmissiveMapper.h"
#include "RenderTargetDrawer.h"
#include "ResourceManager.h"
#include "Shared_RenderingData.h"
#include "Effects.h"
#include "DX11Core.h"
#include "ObjectMeshData.h"
#include "RenderStates.h"
#include "Vertex.h"
#include "RenderTarget.h"
#include "Mesh.h"
#include "EColors.h"

#include <assert.h>

EmissiveMapper::EmissiveMapper(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm)
	: m_DX11Core(dx11Core), m_ResourceManager(rm)
{

}

EmissiveMapper::~EmissiveMapper()
{

}

void EmissiveMapper::PrePass(RenderTarget* output, Shared_RenderingData* dataForRender)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_dc->RSSetState(RenderStates::GetSolidRS());

	_dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));
	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	_dc->OMSetRenderTargets(1, &rt[0], m_DX11Core->GetDSV());

	for (const auto& it : dataForRender->m_ObjectDataForRenderVec)
	{
		// 오브젝트의 메쉬 데이터를 ID를 통해 리소스 매니저로부터 가져온다.
		ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(it->m_Name);

		if (_objMeshData == nullptr)
		{
			continue;
		}

		ID3D11ShaderResourceView* _emissiveMap = m_ResourceManager->GetEmissiveMap(it->m_DiffuseMapName);

		// Emissive Map이 없다면 여기 있을 이유 X
		//if (_emissiveMap == nullptr)
		//{
		//	continue;
		//}
		
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
		_dc->IASetInputLayout(_inputLayout);

		UINT offset = 0;
		_dc->IASetVertexBuffers(0, 1, _objMeshData->m_VertexBuffer.GetAddressOf(), &_objMeshData->m_Stride, &offset);
		_dc->IASetIndexBuffer(_objMeshData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		/// Set Active Tech
		ID3DX11EffectTechnique* _activeTech = nullptr;

		switch (_objMeshData->m_ObjectMeshInfo.m_EffectType)
		{
		case ObjectMeshInfo::eEffectType::Basic:
			_activeTech = Effects::EmissiveFX->BasicTech;
			break;
		case ObjectMeshInfo::eEffectType::NormalMap:
			_activeTech = Effects::EmissiveFX->NormalMapTech;
			break;
		case ObjectMeshInfo::eEffectType::Skinning:
			_activeTech = Effects::EmissiveFX->SkinningTech;
			break;
		}

		// 본이 있는 메쉬이고, 본 트랜스폼이 제대로 들어있으면
		if (_objMeshData->m_BoneFinalTransformVec.size() > 0)
		{
			Effects::EmissiveFX->SetBoneTransforms(_objMeshData->m_BoneFinalTransformVec);
		}

		Effects::EmissiveFX->SetEmissiveMap(_emissiveMap);

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

				Effects::EmissiveFX->SetWorldViewProj(worldViewProj);

				_activeTech->GetPassByIndex(p)->Apply(0, _dc);
				_dc->DrawIndexed(_objMeshData->m_MeshVec[m]->IndexCount,
					_objMeshData->m_MeshVec[m]->StartIndex,
					0);
			}
		}
	}
}

void EmissiveMapper::Mix(RenderTarget* output, RenderTarget* lastRenderTarget, RenderTarget* preEmissiveOutput)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	_dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Blue));
	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	_dc->OMSetRenderTargets(1, &rt[0], m_DX11Core->GetDSV());

	Effects::EmissiveFX->SetFinal(lastRenderTarget->GetSRV());
	Effects::EmissiveFX->SetEmissiveBluredOutput(preEmissiveOutput->GetSRV());
	Effects::EmissiveFX->SetWorldViewProj(EMath::Matrix::Identity);

	ID3DX11EffectTechnique* _tech = Effects::EmissiveFX->MixTech;

	RenderTargetDrawer::DrawRenderTarget(_dc, _tech);
}
