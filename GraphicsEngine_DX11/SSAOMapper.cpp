#include "SSAOMapper.h"
#include "RenderTarget.h"
#include "DeferredLightingManager.h"
#include "MacroDefine.h"
#include "ClientSize.h"
#include "RenderTargetDrawer.h"
#include "ErrChecker.h"
#include "Mathhelper.h"
#include "DX11Core.h"

#include "EColors.h"

#include "Vertex.h"
#include "RenderStates.h"
#include "ObjectMeshData.h"

// For XMCOLOR
#include <directxpackedvector.h>
using namespace DirectX::PackedVector;

SSAOMapper::SSAOMapper(std::shared_ptr<DX11Core> dx11Core, float fovY, float farZ)
	: m_DX11Core(dx11Core), m_RandomVectorSRV(nullptr),
	mAmbientRTV0(nullptr), mAmbientSRV0(nullptr), mAmbientRTV1(nullptr), mAmbientSRV1(nullptr),
	IsActive(true)
{
	m_Width = ClientSize::GetWidth();
	m_Height = ClientSize::GetHeight();

	OnResize(fovY, farZ);

	BuildOffsetVectors();
	BuildRandomVectorTexture();
}

SSAOMapper::~SSAOMapper()
{
	m_RandomVectorSRV = nullptr;

	ReleaseTextureViews();
}

void SSAOMapper::OnResize(float fovY, float farZ)
{
	// we render to ambient map at half the resolution
	// ambient map(blur 쉐이더에 보낼)은 1/4(가로 1/2, 세로 1/2) 크기로 렌더함
	m_AmbientViewport.TopLeftX = 0.0f;
	m_AmbientViewport.TopLeftY = 0.0f;
	m_AmbientViewport.Width = m_Width / 2.0f;
	m_AmbientViewport.Height = m_Height / 2.0f;
	m_AmbientViewport.MinDepth = 0.0f;
	m_AmbientViewport.MaxDepth = 1.0f;

	BuildFrustumFarCorners(fovY, farZ);
	BuildTextureViews();
}

void SSAOMapper::ComputeSSAO(const EMath::Matrix& view, const EMath::Matrix& proj)
{
	ID3D11DeviceContext* dc = m_DX11Core->GetDC();

	// Bind the ambient map as the render target. Observe that this pass does not bind
	// a depth/stencil buffer--it does not need it, and without one, no depth test is
	// performed, which is what we want.
	ID3D11RenderTargetView* renderTargets[1] = { mAmbientRTV0.Get() };
	dc->OMSetRenderTargets(1, renderTargets, 0);
	dc->ClearRenderTargetView(mAmbientRTV0.Get(), reinterpret_cast<const float*>(&EColors::Blue));
	dc->RSSetViewports(1, &m_AmbientViewport);

	// Transform NDC space [-1, +1]^2 to texture space [0, 1]^2
	static const EMath::Matrix T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	EMath::Matrix V = view;
	EMath::Matrix P = proj;
	EMath::Matrix PT = P * T;

	Effects::SSAOFX->SetView(V);
	Effects::SSAOFX->SetViewToTexSpace(PT);
	Effects::SSAOFX->SetOffsetVectors(m_Offsets);
	Effects::SSAOFX->SetFrustumCorners(m_FrustumFarCorner);

	Effects::SSAOFX->SetRandomVecMap(m_RandomVectorSRV.Get());

	RenderTargetDrawer::DrawRenderTarget(dc, Effects::SSAOFX->SSAOTech);
}

void SSAOMapper::Blur(int blurCount)
{
	for (size_t i = 0; i < blurCount; ++i)
	{
		Blur(mAmbientSRV0.Get(), mAmbientRTV1.Get(), true);
		Blur(mAmbientSRV1.Get(), mAmbientRTV0.Get(), false);
	}

	// ComputeSSAO 함수에서 SetViewport로 메인 뷰포트에서 SSAO 전용 뷰포트로 바꿨던 것을 다시 메인 뷰포트로 바꿔준다.
	m_DX11Core->SetMainViewport();
}

void SSAOMapper::Blur(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV, bool horzBlur)
{
	ID3D11DeviceContext* dc = m_DX11Core->GetDC();

	ID3D11RenderTargetView* renderTargets[1] = { outputRTV };
	dc->OMSetRenderTargets(1, renderTargets, 0);
	dc->ClearRenderTargetView(outputRTV, reinterpret_cast<const float*>(&EColors::Red));
	dc->RSSetViewports(1, &m_AmbientViewport);

	Effects::SSAOBlurFX->SetTexelWidth(1.0f / m_AmbientViewport.Width);
	Effects::SSAOBlurFX->SetTexelHeight(1.0f / m_AmbientViewport.Height);
	Effects::SSAOBlurFX->SetInputImage(inputSRV);

	ID3DX11EffectTechnique* tech = Effects::SSAOBlurFX->VertBlurTech;
	if (horzBlur)
		tech = Effects::SSAOBlurFX->HorzBlurTech;

	//RenderTargetDrawer::DrawRenderTarget(tech);

	// Unbind the input SRV as it is going to be an output in the next blur.
	//Effects::SSAOBlurFX->SetInputImage(0);

	/// 원본
	UINT stride = sizeof(Vertex::PosNormalTex);
	UINT offset = 0;

	dc->IASetInputLayout(InputLayouts::PosNormalTex);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dc->IASetVertexBuffers(0, 1, RenderTargetDrawer::GetFullScreenQuad()->m_VertexBuffer.GetAddressOf(), &stride, &offset);
	dc->IASetIndexBuffer(RenderTargetDrawer::GetFullScreenQuad()->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		tech->GetPassByIndex(p)->Apply(0, dc);
		dc->DrawIndexed(6, 0, 0);

		// Unbind the input SRV as it is going to be an output in the next blur.
		Effects::SSAOBlurFX->SetInputImage(0);
		tech->GetPassByIndex(p)->Apply(0, dc);
	}
}

void SSAOMapper::BuildFrustumFarCorners(float fovY, float farZ)
{
	float aspect = (float)m_Width / (float)m_Height;

	float halfHeight = farZ * tanf(0.5f * fovY);
	float halfWidth = aspect * halfHeight;

	m_FrustumFarCorner[0] = EMath::FLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	m_FrustumFarCorner[1] = EMath::FLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	m_FrustumFarCorner[2] = EMath::FLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	m_FrustumFarCorner[3] = EMath::FLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);
}

void SSAOMapper::BuildTextureViews()
{
	ReleaseTextureViews();

	ID3D11Device* device = m_DX11Core->GetDevice();

	// Render ambient map at half resolution.
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = m_Width / 2;
	texDesc.Height = m_Height / 2;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	ID3D11Texture2D* ambientTex0 = 0;
	HR(device->CreateTexture2D(&texDesc, 0, &ambientTex0));
	HR(device->CreateShaderResourceView(ambientTex0, 0, mAmbientSRV0.GetAddressOf()));
	HR(device->CreateRenderTargetView(ambientTex0, 0, mAmbientRTV0.GetAddressOf()));

	ID3D11Texture2D* ambientTex1 = 0;
	HR(device->CreateTexture2D(&texDesc, 0, &ambientTex1));
	HR(device->CreateShaderResourceView(ambientTex1, 0, mAmbientSRV1.GetAddressOf()));
	HR(device->CreateRenderTargetView(ambientTex1, 0, mAmbientRTV1.GetAddressOf()));

	// view saves a reference.
	ReleaseCOM(ambientTex0);
	ReleaseCOM(ambientTex1);
}

void SSAOMapper::ReleaseTextureViews()
{
	mAmbientRTV0 = nullptr;
	mAmbientSRV0 = nullptr;

	mAmbientRTV1 = nullptr;
	mAmbientSRV1 = nullptr;
}

void SSAOMapper::BuildRandomVectorTexture()
{
	ID3D11Device* device = m_DX11Core->GetDevice();

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.SysMemPitch = 256 * sizeof(XMCOLOR);

	XMCOLOR color[256 * 256];
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			EMath::FLOAT3 v(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF());

			color[i * 256 + j] = XMCOLOR(v.x, v.y, v.z, 0.0f);
		}
	}

	initData.pSysMem = color;

	ID3D11Texture2D* tex = 0;
	HR(device->CreateTexture2D(&texDesc, &initData, &tex));

	HR(device->CreateShaderResourceView(tex, 0, m_RandomVectorSRV.GetAddressOf()));

	// view saves a reference.
	ReleaseCOM(tex);
}

void SSAOMapper::BuildOffsetVectors()
{
	// 14개의 고르게 분포된 벡터들로 시작한다.
	// 입방체(큐브)의 여덟 꼭짓점과 각 면의 중점 여섯 개를 그러한 벡터들로 간주한다.
	// 이들을, 이웃한 두 원소가 서로 입방체의 반대쪽에 있도록 하는 순서로 저장해 둔다.
	// 이렇게 하면 표본 수가 14개 미만이라고 해도 항상 고르게 분포된 벡터들을 얻게 된다.

	// 입방체(큐브) 꼭짓점 여덟 개
	m_Offsets[0] = EMath::FLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	m_Offsets[1] = EMath::FLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	m_Offsets[2] = EMath::FLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	m_Offsets[3] = EMath::FLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	m_Offsets[4] = EMath::FLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	m_Offsets[5] = EMath::FLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	m_Offsets[6] = EMath::FLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	m_Offsets[7] = EMath::FLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 입방체(큐브) 면 중점 여섯 개
	m_Offsets[8] = EMath::FLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	m_Offsets[9] = EMath::FLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	m_Offsets[10] = EMath::FLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	m_Offsets[11] = EMath::FLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	m_Offsets[12] = EMath::FLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	m_Offsets[13] = EMath::FLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (UINT i = 0; i < 14; ++i)
	{
		// [0.25, 1.0] 구간의 무작위 길이를 생성한다.
		//float s = MathHelper::RandF(0.25f, 1.0f);
		float s = MathHelper::RandF(0.1f, 0.2f);

		m_Offsets[i].Normalize();
		EMath::Vector4 v = s * m_Offsets[i];

		m_Offsets[i] = v;
	}
}

void SSAOMapper::DrawFullScreenQuad(bool isSSAO)
{
	if (isSSAO)
	{
		EMath::Matrix _s = EMath::Matrix::CreateScale(0.75f, 0.745f, 0.745f);
		EMath::Matrix _t = EMath::Matrix::CreateTranslation(-0.25f, 0.25f, 0.0f);

		EMath::Matrix _world = _s * _t;

		ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewRedTech;
		RenderTargetDrawer::DrawRenderTarget(m_DX11Core->GetDC(), mAmbientSRV0.Get(), _world, Effects::DebugTexFX->ViewRedTech);
	}
	else
	{
		EMath::Matrix world(
			0.245f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.245f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.75f, -0.25f, 0.0f, 1.0f);

		ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewRedTech;
		RenderTargetDrawer::DrawRenderTarget(m_DX11Core->GetDC(), mAmbientSRV0.Get(), world, Effects::DebugTexFX->ViewRedTech);
	}

	// 디버그 텍스트 (Map 이름 출력)
	//DebugManager::GetInstance()->AddNoticeText(TextInfo(XMFLOAT2(0.75f * ClientSize::GetWidth() + 10, ClientSize::GetHeight() * 0.5f + 10),
	//	XMFLOAT4(DirectX::Colors::Black),
	//	(TCHAR*)L"< SSAO Map >"));
}

