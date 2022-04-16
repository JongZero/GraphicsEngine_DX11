#include "DeferredLightingManager.h"
#include "RenderTargetDrawer.h"
#include "DX11Core.h"
#include "RenderTarget.h"
#include "DebugManager.h"
#include "ClientSize.h"
#include "EColors.h"

#include <algorithm>

DeferredLightingManager::DeferredLightingManager(std::shared_ptr<DX11Core> dx11Core)
	: m_DX11Core(dx11Core)
{
	CreateRenderTargets();
}

DeferredLightingManager::~DeferredLightingManager()
{
}

void DeferredLightingManager::PreSSAO()
{
	Effects::SSAOFX->SetNormalDepthRT(m_pNormalDepthRT->GetSRV());
	Effects::SSAOFX->SetDepthRT(m_pShadowRT->GetSRV());
	Effects::SSAOBlurFX->SetNormalDepthRT(m_pNormalDepthRT->GetSRV());
	Effects::SSAOBlurFX->SetDepthRT(m_pShadowRT->GetSRV());
}

void DeferredLightingManager::OnResize()
{
	int width = ClientSize::GetWidth();
	int height = ClientSize::GetHeight();

	for (size_t i = 0; i < m_RenderTargetVec.size(); i++)
	{
		m_RenderTargetVec[i]->OnResize(m_DX11Core->GetDevice(), width, height);
	}

	m_FinalRT->OnResize(m_DX11Core->GetDevice(), width, height);
}

void DeferredLightingManager::SetRenderTargets()
{
	// Set 되어야하는 순서대로 sort 한다
	sort(m_RenderTargetVec.begin(), m_RenderTargetVec.end(),
		[](RenderTarget* rt1, RenderTarget* rt2) -> bool
		{
			return static_cast<int>(rt1->GetOrder()) < static_cast<int>(rt2->GetOrder());
		});

	ID3D11DeviceContext* dc = m_DX11Core->GetDC();

	// Set할 RenderTarget이 담길 vector
	std::vector<ID3D11RenderTargetView*> renderTargets;
	for (const auto& it : m_RenderTargetVec)
	{
		renderTargets.push_back(it->GetRTV());

		/// 디버그 확인
		DebugManager::GetInstance()->AddRenderTarget(it);
	}

	// Clear RenderTargets
	for (const auto& it : renderTargets)
	{
		dc->ClearRenderTargetView(it, reinterpret_cast<const float*>(&EColors::LightYellow));
	}

	dc->OMSetRenderTargets((UINT)renderTargets.size(), &renderTargets[0], m_DX11Core->GetDSV());
}

void DeferredLightingManager::DrawFinalRenderTarget(bool showRenderTargets, bool isSSAO)
{
	if (showRenderTargets)
	{
		if (isSSAO)
		{
			EMath::Matrix world(
				0.245f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.245f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.75f, -0.25f, 0.0f, 1.0f);

			m_FinalRT->SetWorld(world);
		}
		else
		{
			m_FinalRT->SetScale(0.75f, 0.745f);
			m_FinalRT->SetPosition(-0.25f, 0.25f);
		}
	}
	else
	{
		m_FinalRT->SetScale(1, 1);
		m_FinalRT->SetPosition(0, 0);
	}

	m_FinalRT->Draw(m_DX11Core->GetDC());
}

void DeferredLightingManager::DrawRenderTargets()
{
	for (const auto& it : m_RenderTargetVec)
	{
		it->Draw(m_DX11Core->GetDC());
	}

	DrawDepthRenderTarget();
}

void DeferredLightingManager::DrawDepthRenderTarget()
{
	// 원래 디버그 포지션
	EMath::Vector2 originPos = m_pPosHRT->GetPosition();

	// 뎁스를 찍기위한 디버그 포지션
	m_pPosHRT->SetPosition(0.75f, 0.25f);
	m_pPosHRT->SetName(L"< Depth >");

	m_pPosHRT->Draw(m_DX11Core->GetDC(), Effects::DebugTexFX->ViewBlueTech);	// Depth만 찍는다.

	m_pPosHRT->SetPosition(originPos);
	m_pPosHRT->SetName(L"< Pos in NDC >");
}

void DeferredLightingManager::CreateRenderTargets()
{
	int width = ClientSize::GetWidth();
	int height = ClientSize::GetHeight();

	m_FinalRT = std::make_unique<RenderTarget>(width, height, L"< Deferred Lighting >", RenderTargetOrder::None);// ,
		//Vector3(0.75f, 0.745f, 0.745f), Vector3(-0.25f, 0.25f, 0.0f));

	auto albedoRT = std::make_unique<RenderTarget>(width, height, L"< Albedo >", RenderTargetOrder::Albedo);
	m_LightRenderTargetVec.push_back(albedoRT.get());
	m_RenderTargetVec.push_back(albedoRT.get());
	m_UniquePtrRenderTargetVec.push_back(std::move(albedoRT));

	auto posHRT = std::make_unique<RenderTarget>(width, height, L"< Pos in NDC >", RenderTargetOrder::PosH);
	m_pPosHRT = posHRT.get();
	m_LightRenderTargetVec.push_back(posHRT.get());
	m_RenderTargetVec.push_back(posHRT.get());
	m_UniquePtrRenderTargetVec.push_back(std::move(posHRT));

	auto posWRT = std::make_unique<RenderTarget>(width, height, L"< Pos in World >", RenderTargetOrder::PosW);
	m_LightRenderTargetVec.push_back(posWRT.get());
	m_RenderTargetVec.push_back(posWRT.get());
	m_UniquePtrRenderTargetVec.push_back(std::move(posWRT));

	auto materialRT = std::make_unique<RenderTarget>(width, height, L"< Material >", RenderTargetOrder::Material);
	m_LightRenderTargetVec.push_back(materialRT.get());
	m_RenderTargetVec.push_back(materialRT.get());
	m_UniquePtrRenderTargetVec.push_back(std::move(materialRT));

	auto normalWRT = std::make_unique<RenderTarget>(width, height, L"< Normal >", RenderTargetOrder::NormalW);
	m_pNormalWRT = normalWRT.get();
	m_LightRenderTargetVec.push_back(normalWRT.get());
	m_RenderTargetVec.push_back(normalWRT.get());
	m_UniquePtrRenderTargetVec.push_back(std::move(normalWRT));

	auto shadowRT = std::make_unique<RenderTarget>(width, height, L"< Shadow >", RenderTargetOrder::ShadowPosH);
	m_pShadowRT = shadowRT.get();
	m_LightRenderTargetVec.push_back(shadowRT.get());
	m_RenderTargetVec.push_back(shadowRT.get());
	m_UniquePtrRenderTargetVec.push_back(std::move(shadowRT));

	auto normalDepthRT = std::make_unique<RenderTarget>(width, height, L"< NormalDepth >", RenderTargetOrder::NormalDepth);
	m_pNormalDepthRT = normalDepthRT.get();
	m_RenderTargetVec.push_back(normalDepthRT.get());
	m_UniquePtrRenderTargetVec.push_back(std::move(normalDepthRT));
}

void DeferredLightingManager::ComputeDeferredLighting(EMath::Matrix view, EMath::Matrix proj, 
	ID3D11ShaderResourceView* ssaoMap, bool isShadowActive, bool isSsaoActive)
{
	// 순서대로 sort 한다.
	sort(m_LightRenderTargetVec.begin(), m_LightRenderTargetVec.end(),
		[](RenderTarget* rt1, RenderTarget* rt2) -> bool
		{
			return static_cast<int>(rt1->GetOrder()) < static_cast<int>(rt2->GetOrder());
		});

	// 쉐이더에 보낼 SRV
	std::vector<ID3D11ShaderResourceView*> renderTargets;
	for (size_t i = 0; i < m_LightRenderTargetVec.size(); i++)
	{
		renderTargets.push_back(m_LightRenderTargetVec[i]->GetSRV());
	}

	EMath::Matrix viewProj = view * proj;
	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	EMath::Matrix toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	ID3D11DeviceContext* dc = m_DX11Core->GetDC();
	dc->ClearRenderTargetView(m_FinalRT->GetRTV(), reinterpret_cast<const float*>(&EColors::Pink));

	ID3D11RenderTargetView* rt[1] = { m_FinalRT->GetRTV() };
	dc->OMSetRenderTargets(1, rt, m_DX11Core->GetDSV());

	/// Light 갯수, 그래픽 옵션에 따라 분기
	ID3DX11EffectTechnique* tech = nullptr;

	static bool isPBR = false;
	if (GetAsyncKeyState(VK_LCONTROL) & 0x0001)
		isPBR ^= true;
	
	if (!isPBR)
	{
		Effects::DeferredLightingFX->SetRenderTargets(renderTargets);
		Effects::DeferredLightingFX->SetWorldViewProj(EMath::Matrix::Identity);
		Effects::DeferredLightingFX->SetSSAOMap(ssaoMap);
		Effects::DeferredLightingFX->SetViewProjTex(viewProj * toTexSpace);

		//if (isShadowActive && isSsaoActive)
		//	tech = Effects::DeferredLightingFX->LightingShadowSSAOTech;
		//else if (isShadowActive)
		//	tech = Effects::DeferredLightingFX->LightingShadowTech;
		//else if (isSsaoActive)
		//	tech = Effects::DeferredLightingFX->LightingSSAOTech;
		//else
			tech = Effects::DeferredLightingFX->LightingTech;
	}
	else
	{
		Effects::DeferredLightingPBRFX->SetRenderTargets(renderTargets);
		Effects::DeferredLightingPBRFX->SetWorldViewProj(EMath::Matrix::Identity);
		Effects::DeferredLightingPBRFX->SetSSAOMap(ssaoMap);
		Effects::DeferredLightingPBRFX->SetViewProjTex(viewProj * toTexSpace);

		if (isShadowActive && isSsaoActive)
			tech = Effects::DeferredLightingPBRFX->LightingShadowSSAOTech;
		else if (isShadowActive)
			tech = Effects::DeferredLightingPBRFX->LightingShadowTech;
		else if (isSsaoActive)
			tech = Effects::DeferredLightingPBRFX->LightingSSAOTech;
		else
			tech = Effects::DeferredLightingPBRFX->LightingTech;
	}

	// Deferred Lighting Tech로 보낸다.
	m_FinalRT->Draw(dc, tech);
}
