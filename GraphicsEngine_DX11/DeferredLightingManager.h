#pragma once
#include <vector>
#include <d3d11.h>
#include <memory>
#include "EMath.h"

class DX11Core;
class RenderTarget;

class DeferredLightingManager
{
public:
	DeferredLightingManager(std::shared_ptr<DX11Core> dx11Core);
	~DeferredLightingManager();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::unique_ptr<RenderTarget> m_FinalRT;	// 조명 계산이 끝난 최종 RT
	
	RenderTarget* m_pPosHRT;
	RenderTarget* m_pNormalWRT;
	RenderTarget* m_pShadowRT;
	RenderTarget* m_pNormalDepthRT;

	std::vector<std::unique_ptr<RenderTarget>> m_UniquePtrRenderTargetVec;	// 렌더 타겟 유니크 포인터의 소유권을 모아놓음
	std::vector<RenderTarget*> m_LightRenderTargetVec;			// 조명 계산에 쓰일 렌더타겟들
	std::vector<RenderTarget*> m_RenderTargetVec;				// 렌더 타겟들

public:
	RenderTarget* GetPosHRT() { return m_pPosHRT; }
	RenderTarget* GetFinalRT() { return m_FinalRT.get(); }

public:
	void PreSSAO();		// SSAO 쉐이더에 렌더타겟 세팅
	void OnResize();
	void SetRenderTargets();
	void DrawFinalRenderTarget(bool showRenderTargets, bool isSSAO);
	void DrawRenderTargets();
	void ComputeDeferredLighting(EMath::Matrix view, EMath::Matrix proj, ID3D11ShaderResourceView* ssaoMap, bool isShadowActive, bool isSsaoActive);

private:
	void DrawDepthRenderTarget();
	void CreateRenderTargets();
};
