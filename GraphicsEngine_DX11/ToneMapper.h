#pragma once
#include <memory>

class DX11Core;
class RenderTarget;

/// <summary>
/// �� ������ ����Ѵ�.
/// ACES, Reinhard, Simple... ��� �����ؼ� �� ������ �� �ֵ����Ѵ�.
/// 2022. 01. 20 ������
/// </summary>
class ToneMapper
{
public:
	ToneMapper(std::shared_ptr<DX11Core> dx11Core);
	~ToneMapper();

private:
	std::shared_ptr<DX11Core> m_DX11Core;

public:
	enum class Type
	{
		Simple,
		Simple_Invert,
		ACES,
		Reinhard,
	};

public:
	void ToneMap(Type type, RenderTarget* output, RenderTarget* lastRenderTarget);
};

