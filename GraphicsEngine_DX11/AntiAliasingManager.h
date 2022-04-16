#pragma once
#include <memory>

class DX11Core;
class RenderTarget;

/// <summary>
/// Anti-Aliasing을 담당하는 매니저
/// 디퍼드 렌더링에서 라이팅 계산이 끝난 뒤 첫번째 포스트 프로세싱을 하기 전에 보통 수행된다. (후처리 안티에일리어싱)
/// 위의 이유는 포스트 프로세싱을 하면 보통 렌더타겟의 픽셀이 뭉게지기 때문에(Blur, Bloom...),
/// 픽셀이 뭉게지기 전에 오브젝트의 외곽선을 구분하여 그 외곽선에 안티 에일리어싱을 적용해야하기 때문이다.
/// 2021. 11. 24 정종영
/// </summary>
class AntiAliasingManager
{
public:
	AntiAliasingManager(std::shared_ptr<DX11Core> dx11Core);
	~AntiAliasingManager();

private:
	std::shared_ptr<DX11Core> m_DX11Core;

public:
	void StartFXAA(RenderTarget* output, RenderTarget* lastRenderTarget);
};

