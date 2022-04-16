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
/// �׷��Ƚ� ������ ���ϸ� �������� �������� ����ϴ� Ŭ����
/// ���۵� ������(������ ������Ʈ) + ������ ������(���� ������Ʈ)�� �Ѵ�.
/// 2021. 10. 29 ������
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

	// ������ ���� �׸���.
	void PreRender(Shared_RenderingData* dataForRender);
	// MRT�� �ʱ�ȭ�������ϰ� MRT�� ������Ʈ���� ��� �׸���.
	void Render(Shared_RenderingData* dataForRender);
	// SSAO �Ŀ� MRT�� �Էµ� �����͵��� ������ �������� �Ѵ�. ���������� ����Ʈ���μ��� �Ŀ� UI, Text�� �׸��� Present�Ѵ�.
	void PostRender(Shared_RenderingData* dataForRender, ParticleSystemManager* psManager);
};

