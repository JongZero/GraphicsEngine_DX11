#include "DOFDrawer.h"
#include "RenderTargetDrawer.h"
#include "DX11Core.h"
#include "RenderTarget.h"
#include "DebugManager.h"
#include "Shared_RenderingData.h"

#include "EColors.h"

DOFDrawer::DOFDrawer(std::shared_ptr<DX11Core> dx11Core)
	: m_DX11Core(dx11Core)
{

}

DOFDrawer::~DOFDrawer()
{

}

void DOFDrawer::PreDOF(RenderTarget* output, RenderTarget* lastRenderTarget)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	_dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Green));
	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	_dc->OMSetRenderTargets(1, &rt[0], m_DX11Core->GetDSV());

	Effects::DebugTexFX->SetTexture(lastRenderTarget->GetSRV());
	Effects::DebugTexFX->SetWorldViewProj(EMath::Matrix::Identity);

	ID3DX11EffectTechnique* _tech = Effects::DebugTexFX->ViewArgbTech;

	RenderTargetDrawer::DrawRenderTarget(_dc, _tech);
}

void DOFDrawer::DOF(RenderTarget* output, RenderTarget* lastRenderTarget, 
	RenderTarget* blurredRenderTarget, RenderTarget* depthMap, 
	Shared_RenderingData* dataForRender)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	_dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Blue));
	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	_dc->OMSetRenderTargets(1, &rt[0], m_DX11Core->GetDSV());

	Effects::DofFX->SetScreen(lastRenderTarget->GetSRV());
	Effects::DofFX->SetBlurScreen(blurredRenderTarget->GetSRV());
	Effects::DofFX->SetDepthMap(depthMap->GetSRV());
	Effects::DofFX->SetWorldViewProj(EMath::Matrix::Identity);

	static float farStart = 40.0f;
	static float farRange = 60.0f;

	if (GetAsyncKeyState(VK_DOWN) & 0x0001)
	{
		farStart -= 5.0f;
	}

	if (GetAsyncKeyState(VK_UP) & 0x0001)
	{
		farStart += 5.0f;
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x0001)
	{
		farRange -= 5.0f;
	}

	if (GetAsyncKeyState(VK_RIGHT) & 0x0001)
	{
		farRange += 5.0f;
	}

	DebugManager::GetInstance()->AddText(TextInfo(EMath::Vector2(0, 30),
		D2D1::ColorF::Red, (TCHAR*)L"Far Start : %f (ก่ ก้)", farStart));

	DebugManager::GetInstance()->AddText(TextInfo(EMath::Vector2(0, 60),
		D2D1::ColorF::Red, (TCHAR*)L"Far Range : %f (ก็  กๆ)", farRange));

	float fQ = dataForRender->m_FarZ / (dataForRender->m_FarZ - dataForRender->m_NearZ);
	EMath::FLOAT2 projectionValues((-dataForRender->m_NearZ) * fQ, -fQ);
	EMath::FLOAT2 farValues(farStart, 1.0f / max(farRange, 0.001f));

	Effects::DofFX->SetProjValues(projectionValues);
	Effects::DofFX->SetDOFFarValues(farValues);

	ID3DX11EffectTechnique* _tech = Effects::DofFX->DOFTech;
	
	RenderTargetDrawer::DrawRenderTarget(_dc, _tech);
}
