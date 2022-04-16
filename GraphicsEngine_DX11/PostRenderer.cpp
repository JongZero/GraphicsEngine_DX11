#include "PostRenderer.h"
#include "ClientSize.h"
#include "AntiAliasingManager.h"
#include "BlurManager.h"
#include "Shared_RenderingData.h"
#include "ICore.h"
#include "RenderTarget.h"
#include "DebugManager.h"
#include "ToneMapper.h"
#include "EmissiveMapper.h"
#include "DOFDrawer.h"
#include "Scaler.h"
#include "DX11Core.h"



#include "RenderStates.h"



PostRenderer::PostRenderer(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm)
	: m_DX11Core(dx11Core)
{
	int width = ClientSize::GetWidth();
	int height = ClientSize::GetHeight();

	m_AntiAliasingManager = std::make_unique<AntiAliasingManager>(m_DX11Core);
	m_BlurManager = std::make_unique<BlurManager>(m_DX11Core);
	m_OutlineDrawer = std::make_unique<OutlineDrawer>(m_DX11Core, rm);
	m_ToneMapper = std::make_unique<ToneMapper>(m_DX11Core);
	m_EmissiveMapper = std::make_unique<EmissiveMapper>(m_DX11Core, rm);
	m_DOFDrawer = std::make_unique<DOFDrawer>(m_DX11Core);
	m_Scaler = std::make_unique<Scaler>(m_DX11Core, width, height);

	m_ToneMappingSimpleOutput		= std::make_unique<RenderTarget>(width, height, L"< ToneMapping Simple Output >");
	m_ToneMappingOutput				= std::make_unique<RenderTarget>(width, height, L"< ToneMapping Output >");
	m_FXAAOutput					= std::make_unique<RenderTarget>(width, height, L"< FXAA Output >");
	m_OutlineOutput					= std::make_unique<RenderTarget>(width, height, L"< Outline Mix Output >");
	
	m_PreOutlineOutput				= std::make_unique<RenderTarget>(width, height, L"< Pre Outline Output >");

	m_MotionBlurOutput				= std::make_unique<RenderTarget>(width, height, L"< MotionBlur Output >");

	m_PreEmissiveOutput				= std::make_unique<RenderTarget>(width, height, L"< Pre Emissive Output >");
	m_EmissiveOutput				= std::make_unique<RenderTarget>(width, height, L"< Emissive Output >");

	m_PreDOFBlurredOutput			= std::make_unique<RenderTarget>(width / 2, height / 2, L" < Pre DOF Blurred Output >");
	m_PreDOFBlurredUpScaleOutput	= std::make_unique<RenderTarget>(width, height, L" < Pre DOF Blurred Output >");
	m_DOFOutput						= std::make_unique<RenderTarget>(width, height, L"< DOF Output >");

	m_BloomOutput					= std::make_unique<RenderTarget>(width, height, L"< Bloom Output >");
}

PostRenderer::~PostRenderer()
{

}

void PostRenderer::OnResize()
{
	int width = ClientSize::GetWidth();
	int height = ClientSize::GetHeight();

	ID3D11Device* device = m_DX11Core->GetDevice();

	m_BlurManager->OnResize(width, height);
	m_OutlineDrawer->OnResize(width, height);
	m_Scaler->OnResize(width, height);

	// Render Targets
	m_ToneMappingSimpleOutput->OnResize(device, width, height);
	m_ToneMappingOutput->OnResize(device, width, height);
	m_FXAAOutput->OnResize(device, width, height);
	m_OutlineOutput->OnResize(device, width, height);

	m_PreOutlineOutput->OnResize(device, width, height);

	m_MotionBlurOutput->OnResize(device, width, height);

	m_PreEmissiveOutput->OnResize(device, width, height);
	m_EmissiveOutput->OnResize(device, width, height);

	m_PreDOFBlurredOutput->OnResize(device, width / 2, height / 2);
	m_PreDOFBlurredUpScaleOutput->OnResize(device, width, height);
	m_DOFOutput->OnResize(device, width, height);
	
	m_BloomOutput->OnResize(device, width, height);
}

void PostRenderer::PreOutlinePass(Shared_RenderingData* dataForRender, 
	OutlineDrawer::Type type, 
	float size /*= 1.5f*/)
{
	m_OutlineDrawer->PrePass(m_PreOutlineOutput.get(), dataForRender, type, size);
}

void PostRenderer::PreEmissivePass(Shared_RenderingData* dataForRender)
{
	m_EmissiveMapper->PrePass(m_PreEmissiveOutput.get(), dataForRender);
}

void PostRenderer::StartRender(Shared_RenderingData* dataForRender,
	RenderTarget* deferredLightingOutput,
	RenderTarget* depthMap)
{
	m_pFinal = deferredLightingOutput;

	/// Debug Text
	std::wstring onoff;
	static bool dof = false;
	if (GetAsyncKeyState(VK_F1) & 0x0001)
		dof ^= true;

	if (dof)
	{
		onoff = L"DOF : ON (F1)";
	}
	else
	{
		onoff = L"DOF : OFF (F1)";
	}

	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Left, (TCHAR*)onoff.c_str()));

	static bool emissive = false;
	if (GetAsyncKeyState(VK_F2) & 0x0001)
		emissive ^= true;

	if (emissive)
	{
		onoff = L"Emissive : ON (F2)";
	}
	else
	{
		onoff = L"Emissive : OFF (F2)";
	}

	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Left, (TCHAR*)onoff.c_str()));

	static bool tone = true;
	if (GetAsyncKeyState(VK_F3) & 0x0001)
		tone ^= true;

	if (tone)
	{
		onoff = L"ToneMap : ON (F3)";
	}
	else
	{
		onoff = L"ToneMap : OFF (F3)";
	}

	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Left, (TCHAR*)onoff.c_str()));

	static bool outline = true;
	if (GetAsyncKeyState(VK_F4) & 0x0001)
		outline ^= true;

	if (outline)
	{
		onoff = L"Outline : ON (F4)";
	}
	else
	{
		onoff = L"Outline : OFF (F4)";
	}

	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Left, (TCHAR*)onoff.c_str()));

	static bool fxaa = true;
	if (GetAsyncKeyState(VK_F5) & 0x0001)
		fxaa ^= true;

	if (fxaa)
	{
		onoff = L"FXAA : ON (F5)";
	}
	else
	{
		onoff = L"FXAA : OFF (F5)";
	}

	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Left, (TCHAR*)onoff.c_str()));

	static bool motionBlur = false;
	if (GetAsyncKeyState(VK_F6) & 0x0001)
		motionBlur ^= true;

	if (motionBlur)
	{
		onoff = L"Motion Blur : ON (F6)";
	}
	else
	{
		onoff = L"Motion Blur : OFF (F6)";
	}

	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Left, (TCHAR*)onoff.c_str()));

	static bool test = false;
	if (GetAsyncKeyState(VK_F7) & 0x0001)
		test ^= true;

	if (test)
	{
		onoff = L"Scaling : ON (F7)";
	}
	else
	{
		onoff = L"Scaling : OFF (F7)";
	}

	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Left, (TCHAR*)onoff.c_str()));

	static bool bloom = false;
	if (GetAsyncKeyState(VK_F8) & 0x0001)
		bloom ^= true;

	if (bloom)
	{
		onoff = L"Bloom : ON (F8)";
	}
	else
	{
		onoff = L"Bloom : OFF (F8)";
	}

	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Left, (TCHAR*)onoff.c_str()));

	// DOF
	if (dof)
	{
		if (test)
		{
			// Copy & Down Scale
			m_Scaler->Scale(m_PreDOFBlurredOutput.get(), m_pFinal);

			m_BlurManager->Blur(m_PreDOFBlurredOutput.get(), 2.0f, 2);

			ID3D11DeviceContext* _dc = m_DX11Core->GetDC();
			_dc->RSSetViewports(1, m_DX11Core->GetViewport());

			// Up Scale
			m_Scaler->Scale(m_PreDOFBlurredUpScaleOutput.get(), m_PreDOFBlurredOutput.get());

			m_DOFDrawer->DOF(m_DOFOutput.get(), m_pFinal, m_PreDOFBlurredUpScaleOutput.get(), depthMap, dataForRender);
		}
		else
		{
			// Copy
			m_DOFDrawer->PreDOF(m_PreDOFBlurredUpScaleOutput.get(), m_pFinal);

			m_BlurManager->Blur(m_PreDOFBlurredUpScaleOutput.get(), 2.0f, 2);

			m_DOFDrawer->DOF(m_DOFOutput.get(), m_pFinal, m_PreDOFBlurredUpScaleOutput.get(), depthMap, dataForRender);
		}

		m_pFinal = m_DOFOutput.get();
	}

	// Emissive
	if (emissive)
	{
		m_BlurManager->Blur(m_PreEmissiveOutput.get(), 2.0f, 2);

		m_EmissiveMapper->Mix(m_EmissiveOutput.get(), m_pFinal, m_PreEmissiveOutput.get());

		m_pFinal = m_EmissiveOutput.get();
	}

	// Bloom
	if (bloom)
	{
		m_BlurManager->Bloom(m_Scaler.get(), m_BloomOutput.get(), m_pFinal);
		m_pFinal = m_BloomOutput.get();
	}

	// Outline
	if (outline)
	{
		// Outline의 블러 여부
		if (true)
		{
			// PreOutlinePass로 나온 결과를 블러해준다.
			m_BlurManager->Blur(m_PreOutlineOutput.get(), 1.0f, 1);
		}

		m_OutlineDrawer->Draw(m_OutlineOutput.get(), m_pFinal, m_PreOutlineOutput.get());

		m_pFinal = m_OutlineOutput.get();
	}

	// Motion Blur
	if (motionBlur)
	{
		m_BlurManager->MotionBlur(m_MotionBlurOutput.get(), dataForRender, depthMap, m_pFinal);

		m_pFinal = m_MotionBlurOutput.get();
	}

	if (tone)
	{
		m_ToneMapper->ToneMap(ToneMapper::Type::ACES, m_ToneMappingOutput.get(), m_pFinal);
		m_pFinal = m_ToneMappingOutput.get();
	}
	else
	{
		m_ToneMapper->ToneMap(ToneMapper::Type::Simple, m_ToneMappingOutput.get(), m_pFinal);

		m_pFinal = m_ToneMappingOutput.get();
	}

	// FXAA
	if (fxaa)
	{
		m_AntiAliasingManager->StartFXAA(m_FXAAOutput.get(), m_pFinal);
		m_pFinal = m_FXAAOutput.get();
	}
}
