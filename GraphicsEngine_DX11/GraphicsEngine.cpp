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
	
	// FBX에서 읽어들인 데이터를 그래픽스 엔진에서 쓸 수 있는 데이터로 만들고 리소스매니저에 저장한다.
	for (const auto& it : m_ResourceManager->GetFilePathAndFileNameUMap())
	{
		m_MeshBuilder->CreateMeshes(it.second, omi);
	}
}

void GraphicsEngine::OnResize()
{
	m_D2DRenderer->ReleaseRenderTarget();		// 스왑 체인을 리사이즈 하려면 스왑 체인을 참조하고 있는 모든 리소스를 해제하여야한다. D2D의 렌더타겟도 마찬가지다.

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
	// 포인터는 그래픽스엔진 생성 시에 최초로 한 번만 받아오면되도록 수정 예정
	m_pDataForRender = dataForRender;

	// 그래픽스 엔진 내부에 업데이트 다 시킨 후
	// (파티클 시스템, 애니메이션, 등등)
	m_ParticleSystemManager->Update(dataForRender->m_dTime, dataForRender->m_TotalTime);

	// 렌더링을 한다.
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
