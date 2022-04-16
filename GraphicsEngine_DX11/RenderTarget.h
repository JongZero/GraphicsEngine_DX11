#pragma once
#include <string>
#include <d3d11.h>

#include "Effects.h"
#include "RenderTargetOrder.h"
#include "EMath.h"

/// <summary>
/// Client의 사이즈 크기의 렌더타겟
/// SRV, RTV, UAV로 이루어짐
/// </summary>
class RenderTarget
{
public:
	RenderTarget(int width, int height,
		std::wstring name = L"",
		RenderTargetOrder order = RenderTargetOrder::None);
	~RenderTarget();

private:
	std::wstring m_Name;
	RenderTargetOrder m_Order;	// 쉐이더에 렌더타겟이 들어가는 순서, SetRenderTarget 하기 전에 이 변수를 통해 정렬됨

	int m_Width;
	int m_Height;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_UnorderedAccessView;		// Compute Shader

	// For Debug
	EMath::Matrix m_World;				// 최종 World
	EMath::Vector2 m_Scale;				// RenderTarget을 찍을 크기
	EMath::Vector2 m_Position;			// RenderTarget을 찍을 위치

public:
	std::wstring GetName() { return m_Name; }
	void SetName(std::wstring name) { m_Name = name; }

	int GetWidth() { return m_Width; }
	int GetHeight() { return m_Height; }

	RenderTargetOrder GetOrder() { return m_Order; }
	EMath::Matrix GetWorld() { return m_World; }
	void SetWorld(EMath::Matrix world) { m_World = world; }

	ID3D11Texture2D* GetTexture() { return m_Texture.Get(); }
	ID3D11ShaderResourceView* GetSRV() { return m_ShaderResourceView.Get(); }
	ID3D11RenderTargetView* GetRTV() { return m_RenderTargetView.Get(); }
	ID3D11UnorderedAccessView* GetUAV() { return m_UnorderedAccessView.Get(); }

	void SetScale(EMath::Vector2 scale) { m_Scale.x = scale.x; m_Scale.y = scale.y; SetWorld(); }
	void SetScale(float x, float y) { m_Scale.x = x, m_Scale.y = y; SetWorld(); }
	void SetPosition(EMath::Vector2 pos) { m_Position.x = pos.x; m_Position.y = pos.y; SetWorld(); }
	void SetPosition(float x, float y) { m_Position.x = x; m_Position.y = y; SetWorld(); }
	void SetWorld(float scaleX, float scaleY, float posX, float posY)
	{
		m_Scale.x = scaleX; m_Scale.y = scaleY;
		m_Position.x = posX; m_Position.y = posY;
		SetWorld();
	}
	EMath::Vector2 GetPosition() { return m_Position; }

private:
	void SetWorld();

public:
	void OnResize(ID3D11Device* device, int width, int height);
	void Draw(ID3D11DeviceContext* dc, ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewArgbTech);
};
