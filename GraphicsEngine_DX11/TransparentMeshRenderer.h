#pragma once
#include "BaseMeshRenderer.h"
#include <queue>
#include "Vertex.h"
#include <vector>

class TransparentMeshRenderer : public BaseMeshRenderer
{
public:
	TransparentMeshRenderer(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm);
	virtual ~TransparentMeshRenderer();

private:
	std::queue<Shared_ObjectData*> m_RenderQueue;

public:
	void AddRenderQueue(Shared_ObjectData* obj) { m_RenderQueue.push(obj); }

public:
	void Render(Shared_RenderingData* dataForRender, EMath::Matrix& shadowTransform);
	virtual ID3DX11EffectTechnique* Render(const EMath::Matrix& view, const EMath::Matrix& proj,
		const EMath::Vector3& cameraPos, const Shared_ObjectData* objDataForRender, const EMath::Matrix& shadowTransform) override;
};
