#pragma once
#include <wrl.h>
#include <d3d11.h>

/// <summary>
/// Render State ���� Ŭ����
/// </summary>
class RenderStates
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

private:
	static ID3D11RasterizerState* SolidRS;
	static ID3D11RasterizerState* WireframeRS;
	static ID3D11DepthStencilState* EqualsDSS;
	static ID3D11DepthStencilState* DepthDisabledDSS;	// z���۸� ���� ���� ������Ʈ
	static ID3D11BlendState* TransparentBS;				// ���ĸ� ���� ���� ������Ʈ

	static ID3D11RasterizerState* NowRS;			// ���� �����Ǿ��ִ� ���� ������Ʈ

public:
	static ID3D11RasterizerState* GetSolidRS() { return SolidRS; }
	static ID3D11RasterizerState* GetWireframeRS() { return WireframeRS; }
	static void SetNowRS(ID3D11RasterizerState* val) { NowRS = val; }
	static ID3D11RasterizerState* GetNowRS() { return NowRS; }
	static ID3D11DepthStencilState* GetEqualsDSS() { return EqualsDSS; }
	static ID3D11DepthStencilState* GetDepthDisabledDSS() { return DepthDisabledDSS; }
	static ID3D11BlendState* GetTransparentBS() { return TransparentBS; }
};
