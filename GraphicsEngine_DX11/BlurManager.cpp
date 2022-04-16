#include "BlurManager.h"

#include "RenderStates.h"
#include "Effects.h"
#include "DX11Core.h"

#include "RenderTargetDrawer.h"

#include "RenderTarget.h"
#include "MacroDefine.h"

#include "Shared_RenderingData.h"
#include "ClientSize.h"

#include "EColors.h"
#include "Scaler.h"

#include "DebugManager.h"

BlurManager::BlurManager(std::shared_ptr<DX11Core> dx11Core)
	: m_DX11Core(dx11Core), m_pActiveTech(nullptr), m_Width(0), m_Height(0), IsActive(false), m_BloomSize(1.0f)
{
	int width = ClientSize::GetWidth();
	int height = ClientSize::GetHeight();

	m_BlurSetter = std::make_unique<RenderTarget>(width, height, L"Blur Setter");
	m_BlurHelper = std::make_unique<RenderTarget>(width, height, L"Blur Helper");

	m_BloomOriginal = std::make_unique<RenderTarget>(width, height, L"m_BloomOriginal");

	float _scaleWidth = width / 2.0f;
	float _scaleHeight = height / 2.0f;
	m_BloomDownScale4x4_0 = std::make_unique<RenderTarget>((int)_scaleWidth, (int)_scaleHeight, L"m_BloomDownScale4x4_0");
	m_BloomCurve_1 = std::make_unique<RenderTarget>((int)_scaleWidth, (int)_scaleHeight, L"m_BloomCurve_1");

	_scaleWidth /= 3.0f;
	_scaleHeight /= 3.0f;
	m_BloomDownScale6x6_2 = std::make_unique<RenderTarget>((int)_scaleWidth, (int)_scaleHeight, L"m_BloomDownScale6x6_2");

	m_BloomAccumulate_0 = std::make_unique<RenderTarget>((int)_scaleWidth, (int)_scaleHeight, L"m_BloomAccumulate_0");
	m_BloomAccumulate_1 = std::make_unique<RenderTarget>((int)_scaleWidth, (int)_scaleHeight, L"m_BloomAccumulate_1");

	_scaleWidth /= 3.0f;
	_scaleHeight /= 3.0f;
	m_BloomDownScale6x6_3 = std::make_unique<RenderTarget>((int)_scaleWidth, (int)_scaleHeight, L"m_BloomDownScale6x6_3");

	_scaleWidth *= 3.0f;
	_scaleHeight *= 3.0f;
	m_BloomUpScale6x6_4 = std::make_unique<RenderTarget>((int)_scaleWidth, (int)_scaleHeight, L"m_BloomUpScale6x6_4");

	_scaleWidth *= 3.0f;
	_scaleHeight *= 3.0f;
	m_BloomUpScale6x6_5 = std::make_unique<RenderTarget>((int)_scaleWidth, (int)_scaleHeight, L"m_BloomUpScale6x6_5");

	m_BloomAccumulate_2 = std::make_unique<RenderTarget>((int)_scaleWidth, (int)_scaleHeight, L"m_BloomAccumulate_1");
	m_BloomAccumulate_3 = std::make_unique<RenderTarget>((int)_scaleWidth, (int)_scaleHeight, L"m_BloomAccumulate_2");

	_scaleWidth *= 2.0f;
	_scaleHeight *= 2.0f;
	m_BloomUpScale4x4_6 = std::make_unique<RenderTarget>((int)_scaleWidth, (int)_scaleHeight, L"m_BloomUpScale4x4_6");
}

BlurManager::~BlurManager()
{
}

void BlurManager::OnResize(UINT width, UINT height)
{
	m_Width = width;
	m_Height = height;

	ID3D11Device* device = m_DX11Core->GetDevice();

	m_BlurSetter->OnResize(device, width, height);
	m_BlurHelper->OnResize(device, width, height);

	m_BloomOriginal->OnResize(device, width, height);

	float _scaleWidth = width / (2.0f * m_BloomSize);
	float _scaleHeight = height / (2.0f * m_BloomSize);

	m_BloomDownScale4x4_0->OnResize(device, (int)_scaleWidth, (int)_scaleHeight);
	m_BloomCurve_1->OnResize(device, (int)_scaleWidth, (int)_scaleHeight);

	_scaleWidth /= (3.0f * m_BloomSize);
	_scaleHeight /= (3.0f * m_BloomSize);
	m_BloomDownScale6x6_2->OnResize(device, (int)_scaleWidth, (int)_scaleHeight);

	m_BloomAccumulate_0->OnResize(device, (int)_scaleWidth, (int)_scaleHeight);
	m_BloomAccumulate_1->OnResize(device, (int)_scaleWidth, (int)_scaleHeight);

	_scaleWidth /= (3.0f * m_BloomSize);
	_scaleHeight /= (3.0f * m_BloomSize);
	m_BloomDownScale6x6_3->OnResize(device, (int)_scaleWidth, (int)_scaleHeight);

	_scaleWidth *= (3.0f * m_BloomSize);
	_scaleHeight *= (3.0f * m_BloomSize);
	m_BloomUpScale6x6_4->OnResize(device, (int)_scaleWidth, (int)_scaleHeight);

	_scaleWidth *= (3.0f * m_BloomSize);
	_scaleHeight *= (3.0f * m_BloomSize);
	m_BloomUpScale6x6_5->OnResize(device, (int)_scaleWidth, (int)_scaleHeight);

	m_BloomAccumulate_2->OnResize(device, (int)_scaleWidth, (int)_scaleHeight);
	m_BloomAccumulate_3->OnResize(device, (int)_scaleWidth, (int)_scaleHeight);

	_scaleWidth *= (2.0f * m_BloomSize);
	_scaleHeight *= (2.0f * m_BloomSize);
	m_BloomUpScale4x4_6->OnResize(device, (int)_scaleWidth, (int)_scaleHeight);

}

void BlurManager::Blur(RenderTarget* input, float weights, int blurCount)
{
	ID3D11DeviceContext* dc = m_DX11Core->GetDC();
	
	if (input->GetWidth() != m_BlurSetter->GetWidth()
		|| input->GetHeight() != m_BlurSetter->GetHeight())
	{
		m_Width = input->GetWidth();
		m_Height = input->GetHeight();

		m_BlurSetter->OnResize(m_DX11Core->GetDevice(), m_Width, m_Height);
		m_BlurHelper->OnResize(m_DX11Core->GetDevice(), m_Width, m_Height);
	}

	ID3D11RenderTargetView* renderTargets[1] = { m_BlurSetter->GetRTV() };
	dc->OMSetRenderTargets(1, renderTargets, 0);

	dc->ClearRenderTargetView(m_BlurSetter->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));
	dc->ClearRenderTargetView(m_BlurHelper->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	//SetGaussianWeights(weights);

	// Run the compute shader to blur the offscreen texture.
	for (int i = 0; i < blurCount; ++i)
	{
		// HORIZONTAL blur pass.
		D3DX11_TECHNIQUE_DESC techDesc;
		Effects::BlurFX->HorzBlurTech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::BlurFX->SetInputMap(input->GetSRV());
			Effects::BlurFX->SetOutputMap(m_BlurHelper->GetUAV());
			Effects::BlurFX->HorzBlurTech->GetPassByIndex(p)->Apply(0, dc);

			// How many groups do we need to dispatch to cover a row of pixels, where each
			// group covers 256 pixels (the 256 is defined in the ComputeShader).
			UINT numGroupsX = (UINT)ceilf(m_Width / 256.0f);
			dc->Dispatch(numGroupsX, m_Height, 1);
		}

		// Unbind the input texture from the CS for good housekeeping.
		ID3D11ShaderResourceView* nullSRV[1] = { 0 };
		dc->CSSetShaderResources(0, 1, nullSRV);

		// Unbind output from compute shader (we are going to use this output as an input in the next pass, 
		// and a resource cannot be both an output and input at the same time.
		ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
		dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

		// VERTICAL blur pass.
		Effects::BlurFX->VertBlurTech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::BlurFX->SetInputMap(m_BlurHelper->GetSRV());
			Effects::BlurFX->SetOutputMap(input->GetUAV());
			Effects::BlurFX->VertBlurTech->GetPassByIndex(p)->Apply(0, dc);

			// How many groups do we need to dispatch to cover a column of pixels, where each
			// group covers 256 pixels  (the 256 is defined in the ComputeShader).
			UINT numGroupsY = (UINT)ceilf(m_Height / 256.0f);
			dc->Dispatch(m_Width, numGroupsY, 1);
		}

		dc->CSSetShaderResources(0, 1, nullSRV);
		dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	}

	// Disable compute shader.
	dc->CSSetShader(0, 0, 0);
}

void BlurManager::SetGaussianWeights(float sigma)
{
	if (sigma >= 1.0f)
	{
		float weights[9];

		float d = 2.0f * sigma * sigma;

		float sum = 0.0f;
		for (int i = 0; i < 9; ++i)
		{
			float x = (float)i;
			weights[i] = expf(-x * x / d);

			sum += weights[i];
		}

		// Divide by the sum so all the weights add up to 1.0.
		for (int i = 0; i < 9; ++i)
		{
			weights[i] /= sum;
		}

		Effects::BlurFX->SetWeights(weights);
	}
	else
	{
		float weights[11] =
		{
			0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
		};

		Effects::BlurFX->SetWeights(weights);
	}
}

void BlurManager::Bloom(Scaler* scaler, RenderTarget* output, RenderTarget* input)
{
	// 먼저, 원본을 저장 (여기다 계속 픽셀을 누적시킬 것임)
	CopyOriginalRenderTarget(m_BloomOriginal.get(), input);
	CopyOriginalRenderTarget(m_BloomAccumulate_3.get(), input);

	// 4x4 DownSample + Bloom Curve 적용으로 밝은 부분 추출
	scaler->Scale(m_BloomDownScale4x4_0.get(), input);
	BloomCurve(m_BloomCurve_1.get(), m_BloomDownScale4x4_0.get());

	CopyOriginalRenderTarget(m_BloomAccumulate_2.get(), m_BloomCurve_1.get());

	// 6x6 DownSample
	scaler->Scale(m_BloomDownScale6x6_2.get(), m_BloomCurve_1.get());

	CopyOriginalRenderTarget(m_BloomAccumulate_0.get(), m_BloomDownScale6x6_2.get());

	// 6x6 DownSample
	scaler->Scale(m_BloomDownScale6x6_3.get(), m_BloomDownScale6x6_2.get());

	// Blur Again
	Blur(m_BloomDownScale6x6_3.get());

	/// Up Sampling
	// 6x6 UpSample + Accumulate(축적)
	scaler->Scale(m_BloomUpScale6x6_4.get(), m_BloomDownScale6x6_3.get());
	Accumulate(m_BloomAccumulate_1.get(), m_BloomAccumulate_0.get(), m_BloomUpScale6x6_4.get());

	// Blur
	Blur(m_BloomAccumulate_1.get());

	// 6x6 UpSample + Accumulate(축적) + Blur
	scaler->Scale(m_BloomUpScale6x6_5.get(), m_BloomAccumulate_1.get());

	Accumulate(m_BloomAccumulate_3.get(), m_BloomAccumulate_2.get(), m_BloomUpScale6x6_5.get());

	Blur(m_BloomAccumulate_3.get());

	// 4x4 UpSample + Accumulate(축적)
	scaler->Scale(m_BloomUpScale4x4_6.get(), m_BloomAccumulate_3.get());

	Accumulate_Per(output, m_BloomOriginal.get(), m_BloomUpScale4x4_6.get());
}

void BlurManager::CopyOriginalRenderTarget(RenderTarget* output, RenderTarget* input)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	_dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	_dc->OMSetRenderTargets(1, &rt[0], 0);

	Effects::DebugTexFX->SetWorldViewProj(EMath::Matrix::Identity);
	Effects::DebugTexFX->SetTexture(input->GetSRV());

	ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewArgbTech;
	RenderTargetDrawer::DrawRenderTarget(_dc, tech);

	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	_dc->PSSetShaderResources(0, 1, nullSRV);
}

void BlurManager::BloomCurve(RenderTarget* output, RenderTarget* input)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	_dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	_dc->OMSetRenderTargets(1, &rt[0], 0);

	Effects::BloomFX->SetWorldViewProj(EMath::Matrix::Identity);
	Effects::BloomFX->SetTexture(input->GetSRV());

	static float threshold = 1.26f;

	if (GetAsyncKeyState('N') & 0x0001)
	{
		threshold -= 0.1f;
	}
	if (GetAsyncKeyState('M') & 0x0001)
	{
		threshold += 0.1f;
	}
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Left, (TCHAR*)L"%f", threshold));

	Effects::BloomFX->SetThreshold(threshold);

	ID3DX11EffectTechnique* tech = Effects::BloomFX->BloomCurveTech;
	RenderTargetDrawer::DrawRenderTarget(_dc, tech);
}

void BlurManager::Accumulate(RenderTarget* output, RenderTarget* original, RenderTarget* input)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	_dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	_dc->OMSetRenderTargets(1, &rt[0], 0);

	Effects::MixTextureFX->SetWorldViewProj(EMath::Matrix::Identity);

	Effects::MixTextureFX->SetTexture_1(original->GetSRV());
	Effects::MixTextureFX->SetTexture_2(input->GetSRV());

	ID3DX11EffectTechnique* tech = Effects::MixTextureFX->MixTexture_AccumulateTech;
	RenderTargetDrawer::DrawRenderTarget(_dc, tech);

	ID3D11ShaderResourceView* nullSRV[2] = { 0 };
	_dc->PSSetShaderResources(0, 2, nullSRV);
}

void BlurManager::Accumulate_Per(RenderTarget* output, RenderTarget* input1, RenderTarget* input2)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	_dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	_dc->OMSetRenderTargets(1, &rt[0], 0);

	Effects::MixTextureFX->SetWorldViewProj(EMath::Matrix::Identity);

	Effects::MixTextureFX->SetTexture_1(input1->GetSRV());
	Effects::MixTextureFX->SetTexture_2(input2->GetSRV());

	static float _per = 0.5f;
	if (GetAsyncKeyState('K') & 0x0001)
	{
		_per -= 0.1f;
	}
	if (GetAsyncKeyState('L') & 0x0001)
	{
		_per += 0.1f;
	}
	DebugManager::GetInstance()->AddText(TextInfo(eTextPositionInfo::Left, (TCHAR*)L"Acc Per : %f", _per));

	Effects::MixTextureFX->SetPer(_per);

	ID3DX11EffectTechnique* tech = Effects::MixTextureFX->MixTexture_AccumulateTech;
	RenderTargetDrawer::DrawRenderTarget(_dc, tech);

	ID3D11ShaderResourceView* nullSRV[2] = { 0 };
	_dc->PSSetShaderResources(0, 2, nullSRV);
}

void BlurManager::MotionBlur(RenderTarget* output, Shared_RenderingData* dataForRender, RenderTarget* depthMap, RenderTarget* final)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();
	
	_dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	_dc->OMSetRenderTargets(1, &rt[0], m_DX11Core->GetDSV());

	Effects::MotionBlurFX->SetWorldViewProj(EMath::Matrix::Identity);

	EMath::Matrix _viewProj = dataForRender->m_View * dataForRender->m_Proj;
	EMath::Matrix _viewProjInv = _viewProj.Invert();

	Effects::MotionBlurFX->SetViewProjInv(_viewProjInv);
	Effects::MotionBlurFX->SetViewProj_Prev(m_ViewProj_Prev);

	m_ViewProj_Prev = _viewProj;

	Effects::MotionBlurFX->SetDepthMap(depthMap->GetSRV());
	Effects::MotionBlurFX->SetFinal(final->GetSRV());

	ID3DX11EffectTechnique* tech = Effects::MotionBlurFX->MotionBlurTech;
	RenderTargetDrawer::DrawRenderTarget(_dc, tech);
}