#pragma once
#include "Shared_RenderingData.h"
#include <memory>

class DX11Core;
class MeshBuilder;
class ResourceManager;
class DeferredLightingManager;
class RenderTargetDrawer;
class MeshRenderer;
class TransparentMeshRenderer;
class ShadowMapper;
class SSAOMapper;
class DebugColorRenderer;
class PostRenderer;
class D2DRenderer;
class ParticleSystemManager;
class Skybox;


struct ObjectMeshData;
struct Shared_ObjectData;

/// <summary>
/// 그래픽스 엔진에 속하며 실질적인 렌더링을 담당하는 클래스
/// 디퍼드 렌더링(불투명 오브젝트) + 포워드 렌더링(투명 오브젝트)을 한다.
/// 2021. 10. 29 정종영
/// </summary>
class Renderer
{
public:
	Renderer(std::shared_ptr<DX11Core> dx11Core, MeshBuilder* pMB, std::shared_ptr<ResourceManager> rm, std::shared_ptr<D2DRenderer> d2dRenderer);
	~Renderer();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<D2DRenderer> m_D2DRenderer;

	std::unique_ptr<DeferredLightingManager> m_DeferredLightingManager;
	std::unique_ptr<RenderTargetDrawer> m_RenderTargetDrawer;

	std::unique_ptr<MeshRenderer> m_MeshRenderer;
	std::unique_ptr<TransparentMeshRenderer> m_TransparentMeshRenderer;
	std::unique_ptr<DebugColorRenderer> m_DebugColorRenderer;
	std::unique_ptr<PostRenderer> m_PostRenderer;

	std::unique_ptr<ShadowMapper> m_ShadowMapper;
	std::unique_ptr<SSAOMapper> m_SSAOMapper;

	std::unique_ptr<Skybox> m_Skybox;

public:
	void OnResize();
	void StartRender(Shared_RenderingData* dataForRender, ParticleSystemManager* psManager);

private:
	void ConvertData(Shared_RenderingData* dataForRender);

	// 쉐도우 맵을 그린다.
	void PreRender(Shared_RenderingData* dataForRender);
	// MRT를 초기화·세팅하고 MRT에 오브젝트들을 모두 그린다.
	void Render(Shared_RenderingData* dataForRender);
	// SSAO 후에 MRT에 입력된 데이터들을 가지고 라이팅을 한다. 마지막으로 포스트프로세싱 후에 UI, Text를 그리고 Present한다.
	void PostRender(Shared_RenderingData* dataForRender, ParticleSystemManager* psManager);
};

