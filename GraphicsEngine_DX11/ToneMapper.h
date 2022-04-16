#pragma once
#include <memory>

class DX11Core;
class RenderTarget;

/// <summary>
/// 톤 매핑을 담당한다.
/// ACES, Reinhard, Simple... 등등 선택해서 톤 매핑할 수 있도록한다.
/// 2022. 01. 20 정종영
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

