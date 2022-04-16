#include "ToneMapper.h"
#include "Effects.h"
#include "DX11Core.h"
#include "RenderTarget.h"
#include "vertex.h"
#include "RenderTargetDrawer.h"
#include "RenderStates.h"
#include "ObjectMeshData.h"
#include "Mesh.h"

#include "EColors.h"

ToneMapper::ToneMapper(std::shared_ptr<DX11Core> dx11Core)
	: m_DX11Core(dx11Core)
{

}

ToneMapper::~ToneMapper()
{

}

void ToneMapper::ToneMap(Type type, RenderTarget* output, RenderTarget* lastRenderTarget)
{
	ID3DX11EffectTechnique* _activeTech = nullptr;

	switch (type)
	{
	case ToneMapper::Type::Simple:
		_activeTech = Effects::ToneMappingFX->ToneMapping_Simple_Tech;
		break;
	case ToneMapper::Type::Simple_Invert:
		_activeTech = Effects::ToneMappingFX->ToneMapping_Simple_Invert_Tech;
		break;
	case ToneMapper::Type::ACES:
		_activeTech = Effects::ToneMappingFX->ToneMapping_ACES_Tech;
		break;
	case ToneMapper::Type::Reinhard:
		_activeTech = Effects::ToneMappingFX->ToneMapping_Reinhard_Tech;
		break;
	}

	ID3D11DeviceContext* dc = m_DX11Core->GetDC();

	// 클리어
	dc->ClearRenderTargetView(output->GetRTV(), reinterpret_cast<const float*>(&EColors::Black));

	// 렌더 타겟 세팅
	ID3D11RenderTargetView* renderTargets[1] = { output->GetRTV() };
	dc->OMSetRenderTargets(1, renderTargets, m_DX11Core->GetDSV());

	// 텍스쳐 세팅
	Effects::ToneMappingFX->SetTexture(lastRenderTarget->GetSRV());
	Effects::ToneMappingFX->SetWorldViewProj(EMath::Matrix::Identity);

	RenderTargetDrawer::DrawRenderTarget(dc, _activeTech);
}
