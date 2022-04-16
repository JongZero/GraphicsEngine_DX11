#pragma once
#include "IGraphicsEngine.h"
#include <memory>

class DX11Core;
class ResourceManager;
class MeshBuilder;
class Renderer;
class Animator;
class D2DRenderer;
class PickChecker;
class ParticleSystemManager;

/// <summary>
/// 그래픽스 엔진 클래스
/// 2021. 10. 25 정종영
/// </summary>
class GraphicsEngine : public IGraphicsEngine
{
public:
	GraphicsEngine(HWND hWnd, int clientWidth, int clientHeight);
	virtual ~GraphicsEngine();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_ResourceManager;
	std::shared_ptr<D2DRenderer> m_D2DRenderer;

	std::unique_ptr<MeshBuilder> m_MeshBuilder;
	std::unique_ptr<Renderer> m_Renderer;
	std::unique_ptr<Animator> m_Animator;
	std::unique_ptr<PickChecker> m_PickChecker;
	std::unique_ptr<ParticleSystemManager> m_ParticleSystemManager;
	Shared_RenderingData* m_pDataForRender;

private:
	bool m_IsEndGraphicsEngineInitialization;

public:
	virtual bool GetIsEndGraphicsEngineInitialization() override { return m_IsEndGraphicsEngineInitialization; }

public:
	/// 리소스
	virtual void LoadAllResourcesCount(std::wstring folderPath, ParserType type) override;
	virtual void LoadAllResources() override;

	/// 창 크기 설정 & 재초기화
	virtual void SetClientSize(int clientWidth, int clientHeight) override;
	virtual void OnResize() override;
	
	/// 메쉬
	virtual void CreateMeshes(std::wstring objName, ObjectMeshInfo& objectMeshInfo) override;
	// 리소스 매니저에 보관되어있는 메쉬 데이터를 삭제
	virtual void DeleteMeshes(std::wstring objName) override;

	/// 애니메이션
	virtual void InitializeAnimation(std::wstring objName, bool& isCompleted) override;
	virtual void UpdateAnimation(std::wstring objName, std::wstring nowMotionName, float dTime, float speed) override;

	/// 렌더링
	virtual void StartRender(Shared_RenderingData* dataForRender) override;

	/// 피킹
	virtual void PickCheck(int x, int y) override;

	/// 파티클 시스템
	virtual void InitializeParticleSystem(Shared_ParticleSystemData* pData) override;
	virtual void ReleaseParticleSystem(std::wstring name) override;
};
