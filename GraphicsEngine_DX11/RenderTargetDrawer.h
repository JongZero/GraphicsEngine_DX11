#pragma once
#include "Effects.h"
#include "EMath.h"

struct ID3D11DeviceContext;
class MeshBuilder;
class ResourceManager;
struct ObjectMeshData;
class RenderTarget;

/// <summary>
/// ȭ���� ���� ä��� Full Screen Quad �޽��� ���α׷��� �� 1���� �����ϸ� �����
/// �̸� �̿��� ���� Ÿ���� �׷��ش�.
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
