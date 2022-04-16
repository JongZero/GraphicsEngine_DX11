#pragma once
#include <memory>

class DX11Core;
class RenderTarget;
struct Shared_RenderingData;

/// <summary>
/// DOF를 담당하는 클래스, Post Renderer에 속한다.
/// 2022. 01. 27 방소연, 정종영
/// </summary>
class DOFDrawer
{
public:
	DOFDrawer(std::shared_ptr<DX11Core> dx11Core);
	~DOFDrawer();

private:
	std::shared_ptr<DX11Core> m_DX11Core;

public:
	void PreDOF(RenderTarget* output, RenderTarget* lastRenderTarget);		// 원본을 복사한 뒤 블러함
	void DOF(RenderTarget* output, RenderTarget* lastRenderTarget, RenderTarget* blurredRenderTarget, 
		RenderTarget* depthMap, Shared_RenderingData* dataForRender);
};

