#pragma once
#include "BaseMeshRenderer.h"

class MeshRenderer : public BaseMeshRenderer
{
public:
	MeshRenderer(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm);
	virtual ~MeshRenderer();

public:
	virtual ID3DX11EffectTechnique* Render(const EMath::Matrix& view, const EMath::Matrix& proj,
		const EMath::Vector3& cameraPos, const Shared_ObjectData* objDataForRender, const EMath::Matrix& shadowTransform) override;
};

