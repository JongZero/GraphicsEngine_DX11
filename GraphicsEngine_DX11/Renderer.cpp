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

	/// ���� ī�޶��� fovY, farZ�� ���÷� �ٲ�ٸ� ���� �������κ��� �� ���� �����ؼ� ����������Ѵ�.
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
	// ���� ������ �����ߴ� ���� �����͵���
	// �׷��Ƚ� �������� ���� ����� �� �ֵ��� ��ȯ�Ѵ�.

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
	// ī�޶�, ����Ʈ �����͸� ���̴��� ����
	Effects::SetEyePos(dataForRender->m_CameraPosition);
	Effects::SetDirectionalLightVec(dataForRender->m_DirectionalLightInfoVec);
	Effects::SetPointLightVec(dataForRender->m_PointLightInfoVec);
	Effects::SetSpotLightVec(dataForRender->m_SpotLightInfoVec);

	m_DX11Core->ClearMainRenderTarget();
	m_DX11Core->ClearDepthBuffer();

	// ����� ��ü ť �ʱ�ȭ
	DebugManager::GetInstance()->Clear();

	m_DX11Core->Debug();

	// ������ ���� �׸���.
	m_ShadowMapper->StartMap(dataForRender, m_DX11Core.get());
}

void Renderer::Render(Shared_RenderingData* dataForRender)
{
	// MRT�� �ʱ�ȭ�ϰ� �����Ѵ�.
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

	// ������Ʈ���� �׸���.
	for (auto& it : dataForRender->m_ObjectDataForRenderVec)
	{
		Effects::BasicFX->SetMetallicFLOAT(metallic);
		Effects::BasicFX->SetRoughnessFLOAT(roughness);

		Effects::NormalMapFX->SetMetallicFLOAT(metallic);
		Effects::NormalMapFX->SetRoughnessFLOAT(roughness);

		// ���İ� �ִٸ� ������ �޽� �������� ������.
		if (it->m_LegacyMaterialData.Diffuse.w < 1.0f)
		{
			m_TransparentMeshRenderer->AddRenderQueue(it);
		}
		// ���İ� ���� ������Ʈ�鸸 ���۵� ���������� �׸���.
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
	
	/// ������
	// ������ ���� ���� ���۸� ����.
	m_DX11Core->GetDC()->OMSetDepthStencilState(RenderStates::GetDepthDisabledDSS(), 1);

	m_DeferredLightingManager->ComputeDeferredLighting(dataForRender->m_View,
		dataForRender->m_Proj, m_SSAOMapper->GetSSAOMap(), true, true);
	
	// ���� ���۸� �ٽ� �Ҵ�.
	m_DX11Core->GetDC()->OMSetDepthStencilState(m_DX11Core->GetNormalDSS(), 1);

	/// ������ ������Ʈ�� �׸���
	//m_TransparentMeshRenderer->Render(m_DataForRender.get(), m_ShadowMapper->GetShadowTransform());
	
	/// ��ƼŬ �ý��� �׸���
	psManager->Draw(dataForRender->m_View, dataForRender->m_Proj);

	m_DX11Core->GetDC()->OMSetDepthStencilState(RenderStates::GetDepthDisabledDSS(), 1);

	/// ����Ʈ ���μ��� ����
	m_PostRenderer->StartRender(dataForRender, m_DeferredLightingManager->GetFinalRT(),
		m_DeferredLightingManager->GetPosHRT());
	
	m_DX11Core->GetDC()->OMSetDepthStencilState(m_DX11Core->GetNormalDSS(), 1);

	/// ������ ������
	/// ����� ������Ʈ�� �׸���
	//m_DebugColorRenderer->StartRender(dataForRender);

	/// ���� ���
	/// ���� ������� ���� ����Ÿ�ٿ� �׸���
	// ����ü�ΰ� ����� ���� ���� Ÿ�ٿ� �׸������� �� ���ش�.
	m_DX11Core->SetMainRenderTarget();
	m_DX11Core->GetDC()->OMSetDepthStencilState(RenderStates::GetDepthDisabledDSS(), 1);

	/// ����� ����
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

	// UI, Text ���
	m_D2DRenderer->BeginRender();

	/// Debug Text ���
	DebugManager::GetInstance()->DrawTexts(m_D2DRenderer.get());

	m_D2DRenderer->EndRender();

	// ���� �ʱ�ȭ
	m_DX11Core->SetDefaultStates();
	m_DX11Core->ClearShaderResources();

	m_DX11Core->Present();
}
