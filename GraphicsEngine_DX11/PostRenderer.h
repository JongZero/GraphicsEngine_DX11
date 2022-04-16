#pragma once
#include <memory>
#include "OutlineDrawer.h"

class DX11Core;
class ResourceManager;
class AntiAliasingManager;
class BlurManager;
class ToneMapper;
class EmissiveMapper;
class RenderTarget;
class DOFDrawer;
class Scaler;
struct Shared_RenderingData;

/// <summary>
/// 안티 앨리어싱, 포스트 프로세싱 등의 후처리를 위한 렌더러
/// 2022. 01. 05 정종영
/// </summary>
class PostRenderer
{
public:
	PostRenderer(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm);
	~PostRenderer();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::unique_ptr<AntiAliasingManager> m_AntiAliasingManager;
	std::unique_ptr<BlurManager> m_BlurManager;
	std::unique_ptr<OutlineDrawer> m_OutlineDrawer;
	std::unique_ptr<ToneMapper> m_ToneMapper;
	std::unique_ptr<EmissiveMapper> m_EmissiveMapper;
	std::unique_ptr<DOFDrawer> m_DOFDrawer;
	std::unique_ptr<Scaler> m_Scaler;

	std::unique_ptr<RenderTarget> m_ToneMappingSimpleOutput;
	std::unique_ptr<RenderTarget> m_ToneMappingOutput;

	std::unique_ptr<RenderTarget> m_FXAAOutput;

	std::unique_ptr<RenderTarget> m_PreOutlineOutput;		// 아웃라인(만) 그려진 렌더 타겟
	std::unique_ptr<RenderTarget> m_OutlineOutput;			// 기존 렌더타겟 + 아웃라인 합성된 렌더 타겟

	std::unique_ptr<RenderTarget> m_MotionBlurOutput;

	std::unique_ptr<RenderTarget> m_PreEmissiveOutput;
	std::unique_ptr<RenderTarget> m_EmissiveOutput;

	std::unique_ptr<RenderTarget> m_PreDOFBlurredOutput;
	std::unique_ptr<RenderTarget> m_PreDOFBlurredUpScaleOutput;
	std::unique_ptr<RenderTarget> m_DOFOutput;

	std::unique_ptr<RenderTarget> m_BloomOutput;

	RenderTarget* m_pFinal;

public:
	RenderTarget* GetFinalRT() { return m_pFinal; }

public:
	void OnResize();

	void PreOutlinePass(Shared_RenderingData* dataForRender,
		OutlineDrawer::Type type, 
		float size = 1.5f);
	void PreEmissivePass(Shared_RenderingData* dataForRender);

	void StartRender(Shared_RenderingData* dataForRender,	// 렌더링 시 필요한 데이터들
		RenderTarget* deferredLightingOutput,				// 라이팅 패스가 끝난 렌더 타겟
		RenderTarget* depthMap);							// PosH, 뎁스가 담겨있음
};
