#include "AntiAliasingManager.h"
#include "RenderTarget.h"
#include "ClientSize.h"
#include "RenderTargetDrawer.h"

#include "EColors.h"

#include "Vertex.h"
#include "RenderTargetDrawer.h"
#include "ObjectMeshData.h"
#include "Mesh.h"
#include "RenderStates.h"
#include "DX11Core.h"

AntiAliasingManager::AntiAliasingManager(std::shared_ptr<DX11Core> dx11Core)
	: m_DX11Core(dx11Core)
{
	
}

AntiAliasingManager::~AntiAliasingManager()
{
	
}

void AntiAliasingManager::StartFXAA(RenderTarget* output, RenderTarget* lastRenderTarget)
{
	ID3D11DeviceContext* dc = m_DX11Core->GetDC();

	// Ŭ����
	dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	// ���� Ÿ�� ����
	ID3D11RenderTargetView* renderTargets[1] = { output->GetRTV() };
	dc->OMSetRenderTargets(1, renderTargets, m_DX11Core->GetDSV());
	
	// �ؽ��� ����
	Effects::FXAAFX->SetTexture(lastRenderTarget->GetSRV());

	// ���̴� ����
	int width = ClientSize::GetWidth();
	int height = ClientSize::GetHeight();
	EMath::FLOAT4 rcp = EMath::FLOAT4(1.0f / width, 1.0f / height, 0.0f, 0.0f);
	Effects::FXAAFX->SetRCPFrame(rcp);
	Effects::FXAAFX->SetWorldViewProj(EMath::Matrix::Identity);
	
	RenderTargetDrawer::DrawRenderTarget(dc, Effects::FXAAFX->FXAATech);
}
