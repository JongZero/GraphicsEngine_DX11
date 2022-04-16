#pragma once
#include <memory>

class DX11Core;
class RenderTarget;
struct D3D11_VIEWPORT;

/// <summary>
/// 1/4, 1/8... �ٿ� ������ �Ǿ��ִ� ���� Ÿ���� Ŭ���̾�Ʈ ����� �´� ũ��� ���������Ѵ�.
/// 2022. 01. 27 ������
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
	int m_Width;	// ȭ���� width
	int m_Height;	// ȭ���� height
	std::unique_ptr<D3D11_VIEWPORT> m_Viewport;
	std::unique_ptr<RenderTarget> m_ScaleHelper;	// Scale �� input���� ������ ����Ÿ���� output�� ������� ���߱� ���� �߰��ٸ� ����

public:
	void OnResize(int width, int height);		// ȭ�� ����� �°� Down, Up Scale�� �Ѵ�.
	void Scale(RenderTarget* output, RenderTarget* input);
};
