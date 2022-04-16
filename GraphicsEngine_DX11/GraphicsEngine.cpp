#include "GraphicsEngine.h"

#include "DX11Core.h"
#include "ResourceManager.h"
#include "MeshBuilder.h"

#include "ClientSize.h"
#include "MacroDefine.h"
#include "Renderer.h"
#include "Animator.h"
#include "GraphicsEngine_D2D.h"
#include "PickChecker.h"
#include "ParticleSystemManager.h"

GraphicsEngine::GraphicsEngine(HWND hWnd, int clientWidth, int clientHeight)
	: m_IsEndGraphicsEngineInitialization(false)
{
	ClientSize::Width = clientWidth;
	ClientSize::Height = clientHeight;

	m_DX11Core = std::make_shared<DX11Core>(hWnd);
	m_IsEndGraphicsEngineInitialization = m_DX11Core->Initialize(hWnd);
	m_D2DRenderer = std::make_shared<D2DRenderer>(hWnd, m_DX11Core->GetSwapChain());
	m_ResourceManager = std::make_shared<ResourceManager>(m_DX11Core);

	m_MeshBuilder = std::make_unique<MeshBuilder>(m_DX11Core, m_ResourceManager);
	m_Renderer = std::make_unique<Renderer>(m_DX11Core, m_MeshBuilder.get(), m_ResourceManager, m_D2DRenderer);
	m_Animator = std::make_unique<Animator>(m_ResourceManager);
	m_PickChecker = std::make_unique<PickChecker>(m_ResourceManager);
	m_ParticleSystemManager = std::make_unique<ParticleSystemManager>(m_DX11Core, m_ResourceManager);

	OnResize();
}

GraphicsEngine::~GraphicsEngine()
{
	m_D2DRenderer->Release();
}

void GraphicsEngine::SetClientSize(int clientWidth, int clientHeight)
{
	ClientSize::Width = clientWidth;
	ClientSize::Height = clientHeight;
}

void GraphicsEngine::LoadAllResourcesCount(std::wstring folderPath, ParserType type)
{
	m_ResourceManager->LoadAllResourcesCount(folderPath, type);
}

void GraphicsEngine::LoadAllResources()
{
	m_ResourceManager->LoadAllResources();

	ObjectMeshInfo omi(ObjectMeshInfo::eMeshType::FBXERJ, ObjectMeshInfo::eRenderStateType::Solid, ObjectMeshInfo::eEffectType::Basic);
	
	// FBX���� �о���� �����͸� �׷��Ƚ� �������� �� �� �ִ� �����ͷ� ����� ���ҽ��Ŵ����� �����Ѵ�.
	for (const auto& it : m_ResourceManager->GetFilePathAndFileNameUMap())
	{
		m_MeshBuilder->CreateMeshes(it.second, omi);
	}
}

void GraphicsEngine::OnResize()
{
	m_D2DRenderer->ReleaseRenderTarget();		// ���� ü���� �������� �Ϸ��� ���� ü���� �����ϰ� �ִ� ��� ���ҽ��� �����Ͽ����Ѵ�. D2D�� ����Ÿ�ٵ� ����������.

	m_DX11Core->OnResize();
	m_Renderer->OnResize();

	m_D2DRenderer->OnResize();
}

void GraphicsEngine::CreateMeshes(std::wstring objName, ObjectMeshInfo& objectMeshInfo)
{
	m_MeshBuilder->CreateMeshes(objName, objectMeshInfo);
}

void GraphicsEngine::DeleteMeshes(std::wstring objName)
{
	m_ResourceManager->DeleteObjectMeshData(objName);
}

void GraphicsEngine::InitializeAnimation(std::wstring objName, bool& isCompleted)
{
	m_Animator->InitializeAnimation(objName, isCompleted);
}

void GraphicsEngine::UpdateAnimation(std::wstring objName, std::wstring nowMotionName, float dTime, float speed)
{
	m_Animator->UpdateAnimation(objName, nowMotionName, dTime, speed);
}

void GraphicsEngine::StartRender(Shared_RenderingData* dataForRender)
{
	// �����ʹ� �׷��Ƚ����� ���� �ÿ� ���ʷ� �� ���� �޾ƿ���ǵ��� ���� ����
	m_pDataForRender = dataForRender;

	// �׷��Ƚ� ���� ���ο� ������Ʈ �� ��Ų ��
	// (��ƼŬ �ý���, �ִϸ��̼�, ���)
	m_ParticleSystemManager->Update(dataForRender->m_dTime, dataForRender->m_TotalTime);

	// �������� �Ѵ�.
	m_Renderer->StartRender(dataForRender, m_ParticleSystemManager.get());
}

void GraphicsEngine::PickCheck(int x, int y)
{
	m_PickChecker->PickCheck(x, y, m_pDataForRender);
}

void GraphicsEngine::InitializeParticleSystem(Shared_ParticleSystemData* pData)
{
	m_ParticleSystemManager->InitializeParticleSystem(pData);
}

void GraphicsEngine::ReleaseParticleSystem(std::wstring name)
{
	m_ParticleSystemManager->ReleaseParticleSystem(name);
}
