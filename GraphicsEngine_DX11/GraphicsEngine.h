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
/// �׷��Ƚ� ���� Ŭ����
/// 2021. 10. 25 ������
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
	/// ���ҽ�
	virtual void LoadAllResourcesCount(std::wstring folderPath, ParserType type) override;
	virtual void LoadAllResources() override;

	/// â ũ�� ���� & ���ʱ�ȭ
	virtual void SetClientSize(int clientWidth, int clientHeight) override;
	virtual void OnResize() override;
	
	/// �޽�
	virtual void CreateMeshes(std::wstring objName, ObjectMeshInfo& objectMeshInfo) override;
	// ���ҽ� �Ŵ����� �����Ǿ��ִ� �޽� �����͸� ����
	virtual void DeleteMeshes(std::wstring objName) override;

	/// �ִϸ��̼�
	virtual void InitializeAnimation(std::wstring objName, bool& isCompleted) override;
	virtual void UpdateAnimation(std::wstring objName, std::wstring nowMotionName, float dTime, float speed) override;

	/// ������
	virtual void StartRender(Shared_RenderingData* dataForRender) override;

	/// ��ŷ
	virtual void PickCheck(int x, int y) override;

	/// ��ƼŬ �ý���
	virtual void InitializeParticleSystem(Shared_ParticleSystemData* pData) override;
	virtual void ReleaseParticleSystem(std::wstring name) override;
};
