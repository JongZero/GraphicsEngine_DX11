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
	std::unique_ptr<RenderTarget> m_FinalRT;	// ���� ����� ���� ���� RT
	
	RenderTarget* m_pPosHRT;
	RenderTarget* m_pNormalWRT;
	RenderTarget* m_pShadowRT;
	RenderTarget* m_pNormalDepthRT;

	std::vector<std::unique_ptr<RenderTarget>> m_UniquePtrRenderTargetVec;	// ���� Ÿ�� ����ũ �������� �������� ��Ƴ���
	std::vector<RenderTarget*> m_LightRenderTargetVec;			// ���� ��꿡 ���� ����Ÿ�ٵ�
	std::vector<RenderTarget*> m_RenderTargetVec;				// ���� Ÿ�ٵ�

public:
	RenderTarget* GetPosHRT() { return m_pPosHRT; }
	RenderTarget* GetFinalRT() { return m_FinalRT.get(); }

public:
	void PreSSAO();		// SSAO ���̴��� ����Ÿ�� ����
	void OnResize();
	void SetRenderTargets();
	void DrawFinalRenderTarget(bool showRenderTargets, bool isSSAO);
	void DrawRenderTargets();
	void ComputeDeferredLighting(EMath::Matrix view, EMath::Matrix proj, ID3D11ShaderResourceView* ssaoMap, bool isShadowActive, bool isSsaoActive);

private:
	void DrawDepthRenderTarget();
	void CreateRenderTargets();
};
