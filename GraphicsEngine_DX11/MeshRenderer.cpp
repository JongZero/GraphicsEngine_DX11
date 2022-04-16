#include "MeshRenderer.h"
#include "Effects.h"
#include "ObjectMeshData.h"
#include "ResourceManager.h"
#include "Mesh.h"
#include "DX11Core.h"

MeshRenderer::MeshRenderer(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm)
	: BaseMeshRenderer(dx11Core, rm)
{

}

MeshRenderer::~MeshRenderer()
{

}

ID3DX11EffectTechnique* MeshRenderer::Render(const EMath::Matrix& view, const EMath::Matrix& proj,
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

			EMath::Matrix _axisTransformMatrix
			(1, 0, 0, 0,
				0, 0, -1, 0,
				0, 1, 0, 0,
				0, 0, 0, 1);
			
			EMath::Matrix _downScale = EMath::Matrix::CreateScale(0.01f);

			///EMath::Matrix meshWorld = _objMeshData->m_MeshVec[m]->WorldTM;
			/// 애니메이션이 있는지 없는지 체크한 후 있으면 WorldTM, 없으면 NodeTM을 쓰거나
			/// 애니메이션이 없더라도 mesh의 WorldTM을 업데이트 해줘야한다. 방법 두 가지 중 선택 *_*

			//EMath::Matrix meshWorld = _objMeshData->m_MeshVec[m]->NodeTM;
			//EMath::Matrix meshWorld = _objMeshData->m_MeshVec[m]->WorldTM * _axisTransformMatrix;
			//meshWorld = meshWorld * _downScale;

			// 애니메이션 데이터 처리할 때
			// 오브젝트의 월드 데이터 (스케일, 로테이션) 넘겨서
			// 애니메이션 TM * 오브젝트의 월드 TM (포지션 뺀?)

			EMath::Matrix meshWorld = _objMeshData->m_MeshVec[m]->WorldTM;

			if (objDataForRender->m_Name != L"James")
			{
				meshWorld = _objMeshData->m_MeshVec[m]->WorldTM;
			}

			meshWorld = meshWorld * objDataForRender->m_World;

			SetAllTM(_objMeshData->m_ObjectMeshInfo.m_EffectType, meshWorld, view, proj, shadowTransform);

			ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

			_activeTech->GetPassByIndex(p)->Apply(0, m_DX11Core->GetDC());

			_dc->DrawIndexed(_objMeshData->m_MeshVec[m]->IndexCount,
				_objMeshData->m_MeshVec[m]->StartIndex,
				0);
		}
	}

	return nullptr;
}
