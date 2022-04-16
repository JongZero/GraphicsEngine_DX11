#pragma once
#include <memory>

class DX11Core;
class RenderTarget;
struct Shared_RenderingData;

/// <summary>
/// DOF�� ����ϴ� Ŭ����, Post Renderer�� ���Ѵ�.
/// 2022. 01. 27 ��ҿ�, ������
/// </summary>
class DOFDrawer
{
public:
	DOFDrawer(std::shared_ptr<DX11Core> dx11Core);
	~DOFDrawer();

private:
	std::shared_ptr<DX11Core> m_DX11Core;

public:
	void PreDOF(RenderTarget* output, RenderTarget* lastRenderTarget);		// ������ ������ �� ����
	void DOF(RenderTarget* output, RenderTarget* lastRenderTarget, RenderTarget* blurredRenderTarget, 
		RenderTarget* depthMap, Shared_RenderingData* dataForRender);
};

