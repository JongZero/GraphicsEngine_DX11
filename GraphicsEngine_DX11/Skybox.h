#pragma once
#include <memory>

class DX11Core;
class MeshBuilder;
class ResourceManager;
struct ObjectMeshData;
class Cubemap;
class RenderTarget;

/// <summary>
/// IBL과 함께 구현
/// 2022. 03. 30 정종영
/// </summary>
class Skybox
{
public:
	Skybox(std::shared_ptr<DX11Core> dx11Core, MeshBuilder* pMeshBuilder, std::shared_ptr<ResourceManager> rm);
	~Skybox();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_ResourceManager;
	std::unique_ptr<Cubemap> m_pCubemap;
	std::unique_ptr<Cubemap> m_pIrradianceMap;
	std::unique_ptr<Cubemap> m_pPreFilterMap;
	std::unique_ptr<RenderTarget> m_pBrdfLUT;
	ObjectMeshData* m_pSkyboxMeshData;

public:
	void Initialize();
	void Render();
};
