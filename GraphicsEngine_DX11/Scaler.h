#pragma once
#include <memory>

class DX11Core;
class RenderTarget;
struct D3D11_VIEWPORT;

/// <summary>
/// 1/4, 1/8... 다운 스케일 되어있는 렌더 타겟을 클라이언트 사이즈에 맞는 크기로 업스케일한다.
/// 2022. 01. 27 정종영
/// </summary>
class Scaler
{
public:
	Scaler(std::shared_ptr<DX11Core> dx11Core, int width, int height);
	~Scaler();

private:
	std::shared_ptr<DX11Core> m_DX11Core;

public:
	enum class Resolution
	{
		_4x4,
		_6x6,
	};

private:
	int m_Width;	// 화면의 width
	int m_Height;	// 화면의 height
	std::unique_ptr<D3D11_VIEWPORT> m_Viewport;
	std::unique_ptr<RenderTarget> m_ScaleHelper;	// Scale 시 input으로 들어오는 렌더타겟을 output의 사이즈와 맞추기 위한 중간다리 역할

public:
	void OnResize(int width, int height);		// 화면 사이즈에 맞게 Down, Up Scale을 한다.
	void Scale(RenderTarget* output, RenderTarget* input);
};
