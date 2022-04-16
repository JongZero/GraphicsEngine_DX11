#pragma once
#include <memory>

class DX11Core;
class RenderTarget;

/// <summary>
/// Anti-Aliasing�� ����ϴ� �Ŵ���
/// ���۵� ���������� ������ ����� ���� �� ù��° ����Ʈ ���μ����� �ϱ� ���� ���� ����ȴ�. (��ó�� ��Ƽ���ϸ����)
/// ���� ������ ����Ʈ ���μ����� �ϸ� ���� ����Ÿ���� �ȼ��� �������� ������(Blur, Bloom...),
/// �ȼ��� �������� ���� ������Ʈ�� �ܰ����� �����Ͽ� �� �ܰ����� ��Ƽ ���ϸ������ �����ؾ��ϱ� �����̴�.
/// 2021. 11. 24 ������
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

