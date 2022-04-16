#include "Renderer.h"
#include "MeshRenderer.h"
#include "TransparentMeshRenderer.h"
#include "DeferredLightingManager.h"
#include "MacroDefine.h"
#include "MathConverter.h"
#include "DX11Core.h"
#include "ShadowMapper.h"
#include "SSAOMapper.h"
#include "RenderTargetDrawer.h"

#include "ClientSize.h"
#include "DebugColorRenderer.h"
#include "PostRenderer.h"
#include "DebugManager.h"
#include "RenderTarget.h"

#include "MeshBuilder.h"
#include "ResourceManager.h"
#include "GraphicsEngine_D2D.h"
#include "ParticleSystemManager.h"
#include "Skybox.h"


#include "MathHelper.h"
#include "Mesh.h"
#include "RenderStates.h"
#include "ParticleSystem.h"

Renderer::Renderer(std::shared_ptr<DX11Core> dx11Core, MeshBuilder* pMB, std::shared_ptr<ResourceManager> rm, std::shared_ptr<D2DRenderer> d2dRenderer)
	: m_DX11Core(dx11Core), m_D2DRenderer(d2dRenderer)
{
	m_DeferredLightingManager = std::make_unique<DeferredLightingManager>(m_DX11Core);
	m_RenderTargetDrawer = std::make_unique<RenderTargetDrawer>(pMB, rm.get());

	m_MeshRenderer = std::make_unique<MeshRenderer>(m_DX11Core, rm);
	m_TransparentMeshRenderer = std::make_unique<TransparentMeshRenderer>(m_DX11Core, rm);
	m_DebugColorRenderer = std::make_unique<DebugColorRenderer>(m_DX11Core, pMB, rm);
	m_PostRenderer = std::make_unique<PostRenderer>(m_DX11Core, rm);

	m_ShadowMapper = std::make_unique<ShadowMapper>(dx11Core, rm, 2048, 2048);

	/// 현재 카메라의 fovY, farZ가 수시로 바뀐다면 게임 엔진으로부터 이 값도 지속해서 가져와줘야한다.
	m_SSAOMapper = std::make_unique<SSAOMapper>(dx11Core, 0.25f * MathHelper::Pi, 1000.0f);

	m_Skybox = std::make_unique<Skybox>(dx11Core, pMB, rm);

	ObjectMeshInfo test(ObjectMeshInfo::eMeshType::Sphere,
		ObjectMeshInfo::eRenderStateType::Solid, ObjectMeshInfo::eEffectType::Basic);
	pMB->CreateMeshes(L"TestBox", test);
}

Renderer::~Renderer()
{
}

void Renderer::OnResize()
{
	m_DeferredLightingManager->OnResize();
	m_SSAOMapper->OnResize(0.25f * MathHelper::Pi, 1000.0f);

	m_PostRenderer->OnResize();
}

void Renderer::StartRender(Shared_RenderingData* dataForRender, ParticleSystemManager* psManager)
{
	ConvertData(dataForRender);

	PreRender(dataForRender);
	Render(dataForRender);
	PostRender(dataForRender, psManager);
}

void Renderer::ConvertData(Shared_RenderingData* dataForRender)
{
	// 게임 엔진과 공유했던 렌더 데이터들을
	// 그래픽스 엔진에서 쉽게 사용할 수 있도록 변환한다.

	/// Object
	for (auto& it : dataForRender->m_ObjectDataForRenderVec)
	{
		if (0 == wcscmp(L"Spain_Background", it->m_Name))
		{
			it->m_World = EMath::Matrix::CreateScale(0.01f);
		}

		if (0 == wcscmp(L"James", it->m_Name))
		{
			it->m_World = EMath::Matrix::CreateScale(0.01f) * EMath::Matrix::CreateTranslation(EMath::Vector3(0, 13.3f, 0));
		}
	}
}

void Renderer::PreRender(Shared_RenderingData* dataForRender)
{
	// 카메라, 라이트 데이터를 쉐이더에 세팅
	Effects::SetEyePos(dataForRender->m_CameraPosition);
	Effects::SetDirectionalLightVec(dataForRender->m_DirectionalLightInfoVec);
	Effects::SetPointLightVec(dataForRender->m_PointLightInfoVec);
	Effects::SetSpotLightVec(dataForRender->m_SpotLightInfoVec);

	m_DX11Core->ClearMainRenderTarget();
	m_DX11Core->ClearDepthBuffer();

	// 디버그 객체 큐 초기화
	DebugManager::GetInstance()->Clear();

	m_DX11Core->Debug();

	// 쉐도우 맵을 그린다.
	m_ShadowMapper->StartMap(dataForRender, m_DX11Core.get());
}

void Renderer::Render(Shared_RenderingData* dataForRender)
{
	// MRT를 초기화하고 세팅한다.
	m_DX11Core->ClearDepthBuffer();
	m_DeferredLightingManager->SetRenderTargets();

	/// metallic, roughness Test
	static float metallic = 0.5f;
	static float roughness = 0.5f;

	if (GetAsyncKeyState('Z') & 0x0001)
		metallic -= 0.1f;
	if (GetAsyncKeyState('X') & 0x0001)
		metallic += 0.1f;
	if (GetAsyncKeyState('C') & 0x0001)
		roughness -= 0.1f;
	if (GetAsyncKeyState('V') & 0x0001)
		roughness += 0.1f;
		
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, (TCHAR*)L"metallic : %f (Z, X)", metallic));
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Right, (TCHAR*)L"roughness : %f (C, V)", roughness));

	// 오브젝트들을 그린다.
	for (auto& it : dataForRender->m_ObjectDataForRenderVec)
	{
		Effects::BasicFX->SetMetallicFLOAT(metallic);
		Effects::BasicFX->SetRoughnessFLOAT(roughness);

		Effects::NormalMapFX->SetMetallicFLOAT(metallic);
		Effects::NormalMapFX->SetRoughnessFLOAT(roughness);

		// 알파가 있다면 투명한 메쉬 렌더러로 보낸다.
		if (it->m_LegacyMaterialData.Diffuse.w < 1.0f)
		{
			m_TransparentMeshRenderer->AddRenderQueue(it);
		}
		// 알파가 없는 오브젝트들만 디퍼드 렌더링으로 그린다.
		else
		{
			m_MeshRenderer->Render(dataForRender->m_View, dataForRender->m_Proj,
				dataForRender->m_CameraPosition, it, m_ShadowMapper->GetShadowTransform());
		}
	}
	
	// Outline Pass
	static OutlineDrawer::Type _t = OutlineDrawer::Type::OverSize;
	m_PostRenderer->PreOutlinePass(dataForRender, _t, 1.0f);

	// Emissive Pass
	m_DX11Core->ClearDepthBuffer();
	m_PostRenderer->PreEmissivePass(dataForRender);
}

void Renderer::PostRender(Shared_RenderingData* dataForRender, ParticleSystemManager* psManager)
{
	/// SSAO
	m_DeferredLightingManager->PreSSAO();
	m_SSAOMapper->ComputeSSAO(dataForRender->m_View, dataForRender->m_Proj);
	m_SSAOMapper->Blur(2);
	
	/// 라이팅
	// 라이팅 전에 뎁스 버퍼를 끈다.
	m_DX11Core->GetDC()->OMSetDepthStencilState(RenderStates::GetDepthDisabledDSS(), 1);

	m_DeferredLightingManager->ComputeDeferredLighting(dataForRender->m_View,
		dataForRender->m_Proj, m_SSAOMapper->GetSSAOMap(), true, true);
	
	// 뎁스 버퍼를 다시 켠다.
	m_DX11Core->GetDC()->OMSetDepthStencilState(m_DX11Core->GetNormalDSS(), 1);

	/// 투명한 오브젝트들 그리기
	//m_TransparentMeshRenderer->Render(m_DataForRender.get(), m_ShadowMapper->GetShadowTransform());
	
	/// 파티클 시스템 그리기
	psManager->Draw(dataForRender->m_View, dataForRender->m_Proj);

	m_DX11Core->GetDC()->OMSetDepthStencilState(RenderStates::GetDepthDisabledDSS(), 1);

	/// 포스트 프로세싱 시작
	m_PostRenderer->StartRender(dataForRender, m_DeferredLightingManager->GetFinalRT(),
		m_DeferredLightingManager->GetPosHRT());
	
	m_DX11Core->GetDC()->OMSetDepthStencilState(m_DX11Core->GetNormalDSS(), 1);

	/// 포워드 렌더링
	/// 디버그 오브젝트들 그리기
	//m_DebugColorRenderer->StartRender(dataForRender);

	/// 최종 출력
	/// 최종 결과물을 메인 렌더타겟에 그리기
	// 스왑체인과 연결된 메인 렌더 타겟에 그리기위해 셋 해준다.
	m_DX11Core->SetMainRenderTarget();
	m_DX11Core->GetDC()->OMSetDepthStencilState(RenderStates::GetDepthDisabledDSS(), 1);

	/// 디버그 관련
	std::wstring onoff;
	static bool isDebugRenderTarget = true;
	if (GetAsyncKeyState(VK_F11) & 0x0001)
		isDebugRenderTarget ^= true;

	if (isDebugRenderTarget)
	{
		onoff = L"Show Debug RenderTargets : ON (F11)";
	}
	else
	{
		onoff = L"Show Debug RenderTargets : OFF (F11)";
	}

	if (isDebugRenderTarget)
	{
		DebugManager::GetInstance()->DrawRenderTargets(m_DX11Core.get(), m_PostRenderer->GetFinalRT());
	}
	else
	{
		m_PostRenderer->GetFinalRT()->SetWorld(1, 1, 0, 0);
		m_PostRenderer->GetFinalRT()->Draw(m_DX11Core->GetDC());
	}

	// UI, Text 출력
	m_D2DRenderer->BeginRender();

	/// Debug Text 출력
	DebugManager::GetInstance()->DrawTexts(m_D2DRenderer.get());

	m_D2DRenderer->EndRender();

	// 세팅 초기화
	m_DX11Core->SetDefaultStates();
	m_DX11Core->ClearShaderResources();

	m_DX11Core->Present();
}
