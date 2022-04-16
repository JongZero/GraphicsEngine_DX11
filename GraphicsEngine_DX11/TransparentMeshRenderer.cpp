#include "TransparentMeshRenderer.h"
#include "Effects.h"
#include "ObjectMeshData.h"
#include "ResourceManager.h"
#include "Mesh.h"
#include "DX11Core.h"
#include "RenderStates.h"



#include "Mathhelper.h"

TransparentMeshRenderer::TransparentMeshRenderer(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm)
	: BaseMeshRenderer(dx11Core, rm)
{

}

TransparentMeshRenderer::~TransparentMeshRenderer()
{

}

void TransparentMeshRenderer::Render(Shared_RenderingData* dataForRender, EMath::Matrix& shadowTransform)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	/// 알파가 있는 오브젝트를 포워드 렌더링으로 그리기 전에
	/// 렌더 하기 전에 카메라로 부터 먼 오브젝트가 먼저 그려지도록 
	/// sort 해야한다.

	// 블렌드 스테이트 설정
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	_dc->OMSetBlendState(RenderStates::GetTransparentBS(), blendFactor, 0xffffffff);

	for (int i = 0; i < m_RenderQueue.size(); i++)
	{
		Shared_ObjectData* objData = m_RenderQueue.front();

		Render(dataForRender->m_View, dataForRender->m_Proj, dataForRender->m_CameraPosition,
			objData, shadowTransform);

		m_RenderQueue.pop();
	}

	// Restore default blend state
	_dc->OMSetBlendState(0, blendFactor, 0xffffffff);
}

ID3DX11EffectTechnique* TransparentMeshRenderer::Render(const EMath::Matrix& view, const EMath::Matrix& proj,
	const EMath::Vector3& cameraPos, const Shared_ObjectData* objDataForRender, const EMath::Matrix& shadowTransform)
{
	ID3DX11EffectTechnique* _activeTech = BaseMeshRenderer::Render(view, proj, cameraPos, objDataForRender, shadowTransform);

	if (_activeTech == nullptr)
	{
		return nullptr;
	}

	// 오브젝트의 메쉬 데이터를 ID를 통해 리소스 매니저로부터 가져온다.
	ObjectMeshData* _objMeshData = m_pResourceManager->GetObjectMeshData(objDataForRender->m_Name);

	// 본이 있는 메쉬이고, 본 트랜스폼이 제대로 들어있으면
	if (_objMeshData->m_BoneFinalTransformVec.size() > 0)
	{
		Effects::SkinningFX->SetBoneTransforms(_objMeshData->m_BoneFinalTransformVec);
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

			meshWorld = meshWorld * objDataForRender->m_World;

			SetAllTM(_objMeshData->m_ObjectMeshInfo.m_EffectType, meshWorld, view, proj, shadowTransform);

			_activeTech->GetPassByIndex(p)->Apply(0, m_DX11Core->GetDC());

			m_DX11Core->GetDC()->DrawIndexed(_objMeshData->m_MeshVec[m]->IndexCount,
				_objMeshData->m_MeshVec[m]->StartIndex,
				0);
		}
	}

	return nullptr;
}
