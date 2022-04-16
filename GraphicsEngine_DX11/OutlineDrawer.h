#pragma once
#include <memory>
#include <wrl.h>

class DX11Core;
class ResourceManager;
class RenderTarget;
struct ID3D11DeviceContext;
struct Shared_RenderingData;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;

/// <summary>
/// 오브젝트의 외곽선을 그린다.
/// 2021. 12. 29 정종영
/// </summary>
class OutlineDrawer
{
public:
	OutlineDrawer(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm);
	~OutlineDrawer();

public:
	enum class Type
	{
		LaplacianFilter,	// 노말 벡터를 이용하고 라플라시안 필터를 이용해 외곽선을 검출 (오브젝트 내부까지 세세히 외곽선이 그려짐)
		OverSize,			// 외곽선을 검출하지 않고 단지 오브젝트의 사이즈를 키워서 렌더링한 후 스텐실 마스킹을 통해 외곽선을 그림 
							// (오브젝트의 내부까지는 외곽선이 그려지진 않으나, 두껍고 깔끔한 외곽선을 그릴 수 있음)
	};
	
private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_ResourceManager;
	ID3D11DeviceContext* m_DC;

	Type m_Type;
	float m_Size;			// 오버 사이즈 타입 시 외곽선 두께

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer_ForDepthMap;			// 뎁스 맵과 바인딩하여 사용
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView_ForDepthMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_DepthMap;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;						// 기존 오브젝트 패스와 독립적으로 뎁스 기록
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

	std::unique_ptr<RenderTarget> m_NormalW;		// 라플라스 필터 시 노말 뽑기용

public:
	void PrePass(RenderTarget* output, Shared_RenderingData* dataForRender, Type type, float size = 1.5f);
	void Draw(RenderTarget* output, RenderTarget* lastRenderTarget, RenderTarget* preOutlineOutput);		// lastRenderTarget + preOutlineOutput
	void OnResize(int width, int height);

private:
	void CreateRenderTargets();
	void OnResizeDepthBuffers(int width, int height);

	void DrawNormal(Shared_RenderingData* dataForRender);
	void DrawOutlineByLaplacianFilter(RenderTarget* output);

	void DrawDepth(Shared_RenderingData* dataForRender);
	void DrawOutlineByOverSize(RenderTarget* output, Shared_RenderingData* dataForRender);
};
