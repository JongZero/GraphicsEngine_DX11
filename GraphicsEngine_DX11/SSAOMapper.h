#pragma once
#include <memory>
#include <d3d11.h>
#include <wrl.h>

#include "EMath.h"

class RenderTarget;
class DeferredLightingManager;
class DX11Core;

class SSAOMapper
{
public:
	SSAOMapper(std::shared_ptr<DX11Core> dx11Core, float fovY, float farZ);
	~SSAOMapper();

public:
	bool IsActive;

private:
	std::shared_ptr<DX11Core> m_DX11Core;

	UINT m_Width;
	UINT m_Height;

	EMath::Vector4 m_FrustumFarCorner[4];
	EMath::Vector4 m_Offsets[14];

	D3D11_VIEWPORT m_AmbientViewport;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_RandomVectorSRV;

	// Need two for ping-ponging during blur.
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mAmbientRTV0;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mAmbientSRV0;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mAmbientRTV1;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mAmbientSRV1;

public:
	ID3D11ShaderResourceView* GetSSAOMap() { return mAmbientSRV0.Get(); }

public:
	void OnResize(float fovY, float farZ);
	void ComputeSSAO(const EMath::Matrix& view, const EMath::Matrix& proj);
	void Blur(int blurCount);
	void DrawFullScreenQuad(bool isSSAO);

private:
	void Blur(ID3D11ShaderResourceView* inputSRV, ID3D11RenderTargetView* outputRTV, bool horzBlur);
	void BuildFrustumFarCorners(float fovY, float farZ);

	void BuildTextureViews();
	void ReleaseTextureViews();

	void BuildRandomVectorTexture();

	void BuildOffsetVectors();
};
