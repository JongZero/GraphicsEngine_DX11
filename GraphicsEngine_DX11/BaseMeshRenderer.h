#pragma once
#include <memory>
#include "ObjectMeshInfo.h"
#include "Shared_RenderingData.h"

class DX11Core;
class ResourceManager;
class ID3DX11EffectTechnique;

/// <summary>
/// MeshRenderer와 SkinnedMeshRenderer의 베이스 클래스
/// 오브젝트 1개의 메쉬들을 그린다.
/// 2021. 10. 29 정종영
/// </summary>
class BaseMeshRenderer
{
public:
	BaseMeshRenderer(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm);
	virtual ~BaseMeshRenderer();

protected:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_pResourceManager;

	enum class RenderType
	{
		Forward,		// 알파 있는 오브젝트는 포워드 렌더링
		Deferred,
	};

public:
	virtual ID3DX11EffectTechnique* Render(const EMath::Matrix& view, const EMath::Matrix& proj,
		const EMath::Vector3& cameraPos, const Shared_ObjectData* objDataForRender, const EMath::Matrix& shadowTransform);

protected:
	void SetAllTM(const ObjectMeshInfo::eEffectType& et, const EMath::Matrix& meshWorld, const EMath::Matrix& view,
		const EMath::Matrix& proj, const EMath::Matrix& shadowTransform);

private:
	ID3DX11EffectTechnique* SetBasicTech(RenderType rt, bool isDiffuseMapExist);
	ID3DX11EffectTechnique* SetNormalMapTech(RenderType rt);
	ID3DX11EffectTechnique* SetSkinningTech(RenderType rt, bool isDiffuseMapExist, bool isNormalMapExist);
};
