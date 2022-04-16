#pragma once
#include <wrl.h>
#include <d3d11.h>

/// <summary>
/// Render State 전역 클래스
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
	static ID3D11DepthStencilState* DepthDisabledDSS;	// z버퍼를 끄기 위한 스테이트
	static ID3D11BlendState* TransparentBS;				// 알파를 위한 블렌드 스테이트

	static ID3D11RasterizerState* NowRS;			// 현재 설정되어있는 렌더 스테이트

public:
	static ID3D11RasterizerState* GetSolidRS() { return SolidRS; }
	static ID3D11RasterizerState* GetWireframeRS() { return WireframeRS; }
	static void SetNowRS(ID3D11RasterizerState* val) { NowRS = val; }
	static ID3D11RasterizerState* GetNowRS() { return NowRS; }
	static ID3D11DepthStencilState* GetEqualsDSS() { return EqualsDSS; }
	static ID3D11DepthStencilState* GetDepthDisabledDSS() { return DepthDisabledDSS; }
	static ID3D11BlendState* GetTransparentBS() { return TransparentBS; }
};
