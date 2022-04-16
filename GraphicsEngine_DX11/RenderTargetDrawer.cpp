#include "RenderTargetDrawer.h"
#include "Vertex.h"
#include "Effects.h"
#include "RenderTarget.h"
#include "RenderStates.h"
#include "ObjectMeshInfo.h"
#include "ObjectMeshData.h"
#include "MeshBuilder.h"
#include "ResourceManager.h"
#include "MacroDefine.h"
#include "Mesh.h"

ObjectMeshData* RenderTargetDrawer::m_pFullScreenQuad = nullptr;

RenderTargetDrawer::RenderTargetDrawer(MeshBuilder* pMeshBuilder, ResourceManager* pRM)
{
	ObjectMeshInfo omi(ObjectMeshInfo::eMeshType::FullScreenQuad, 
		ObjectMeshInfo::eRenderStateType::Solid, ObjectMeshInfo::eEffectType::Basic);
	pMeshBuilder->CreateMeshes(L"FullScreenQuad", omi);

	m_pFullScreenQuad = pRM->GetObjectMeshData(L"FullScreenQuad");
}

RenderTargetDrawer::~RenderTargetDrawer()
{
	
}

void RenderTargetDrawer::DrawRenderTarget(ID3D11DeviceContext* dc,
	RenderTarget* rt,
	ID3DX11EffectTechnique* tech /*= Effects::DebugTexFX->ViewArgbTech*/)
{
	Effects::DebugTexFX->SetWorldViewProj(rt->GetWorld());
	Effects::DebugTexFX->SetTexture(rt->GetSRV());

	DrawRenderTarget(dc, tech);
}

void RenderTargetDrawer::DrawRenderTarget(ID3D11DeviceContext* dc,
	ID3D11ShaderResourceView* srv,
	EMath::Matrix world,
	ID3DX11EffectTechnique* tech /*= Effects::DebugTexFX->ViewArgbTech*/)
{
	Effects::DebugTexFX->SetWorldViewProj(world);
	Effects::DebugTexFX->SetTexture(srv);

	DrawRenderTarget(dc, tech);
}

void RenderTargetDrawer::DrawRenderTarget(ID3D11DeviceContext* dc,
	ID3DX11EffectTechnique* tech)
{
	dc->RSSetState(RenderStates::GetSolidRS());

	dc->IASetInputLayout(InputLayouts::PosNormalTex);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, m_pFullScreenQuad->m_VertexBuffer.GetAddressOf(), &m_pFullScreenQuad->m_Stride, &offset);
	dc->IASetIndexBuffer(m_pFullScreenQuad->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(m_pFullScreenQuad->m_MeshVec[0]->IndexCount,
			m_pFullScreenQuad->m_MeshVec[0]->StartIndex,
			0);
	}
}
