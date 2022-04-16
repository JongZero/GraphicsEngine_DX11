#include "Scaler.h"
#include "RenderTargetDrawer.h"
#include "RenderTarget.h"
#include "ObjectMeshData.h"
#include "Mesh.h"
#include "EColors.h"
#include "DX11Core.h"

Scaler::Scaler(std::shared_ptr<DX11Core> dx11Core, int width, int height)
	: m_DX11Core(dx11Core)
{
	m_Viewport = std::make_unique<D3D11_VIEWPORT>();

	OnResize(width, height);
}

Scaler::~Scaler()
{

}

void Scaler::OnResize(int width, int height)
{
	m_Width = width;
	m_Height = height;

	m_Viewport->TopLeftX = 0.0f;
	m_Viewport->TopLeftY = 0.0f;
	m_Viewport->Width = (float)m_Width;
	m_Viewport->Height = (float)m_Height;
	m_Viewport->MinDepth = 0.0f;
	m_Viewport->MaxDepth = 1.0f;
}

void Scaler::Scale(RenderTarget* output, RenderTarget* input)
{
	m_Viewport->Width = (float)output->GetWidth();
	m_Viewport->Height = (float)output->GetHeight();

	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	_dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	ID3D11RenderTargetView* rt[] = { output->GetRTV() };
	_dc->OMSetRenderTargets(1, &rt[0], 0);

	_dc->RSSetViewports(1, m_Viewport.get());

	Effects::DebugTexFX->SetWorldViewProj(EMath::Matrix::Identity);
	Effects::DebugTexFX->SetTexture(input->GetSRV());

	ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewArgbTech;
	RenderTargetDrawer::DrawRenderTarget(_dc, tech);

	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	_dc->PSSetShaderResources(0, 1, nullSRV);
}
