#pragma once
#include "Effects.h"
#include "EMath.h"

struct ID3D11DeviceContext;
class MeshBuilder;
class ResourceManager;
struct ObjectMeshData;
class RenderTarget;

/// <summary>
/// 화면을 가득 채우는 Full Screen Quad 메쉬는 프로그램에 단 1개만 존재하면 충분함
/// 이를 이용해 렌더 타겟을 그려준다.
/// </summary>
class RenderTargetDrawer
{
public:
	RenderTargetDrawer(MeshBuilder* pMeshBuilder, ResourceManager* pRM);
	~RenderTargetDrawer();

private:
	static ObjectMeshData* m_pFullScreenQuad;

public:
	static ObjectMeshData* GetFullScreenQuad() { return m_pFullScreenQuad; }

public:
	static void DrawRenderTarget(ID3D11DeviceContext* dc,
		RenderTarget* rt,
		ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewArgbTech);
	static void DrawRenderTarget(ID3D11DeviceContext* dc,
		ID3D11ShaderResourceView* srv,
		EMath::Matrix world,
		ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewArgbTech);
	static void DrawRenderTarget(ID3D11DeviceContext* dc,
		ID3DX11EffectTechnique* tech);
};
