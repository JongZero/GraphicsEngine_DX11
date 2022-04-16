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
/// ������Ʈ�� �ܰ����� �׸���.
/// 2021. 12. 29 ������
/// </summary>
class OutlineDrawer
{
public:
	OutlineDrawer(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm);
	~OutlineDrawer();

public:
	enum class Type
	{
		LaplacianFilter,	// �븻 ���͸� �̿��ϰ� ���ö�þ� ���͸� �̿��� �ܰ����� ���� (������Ʈ ���α��� ������ �ܰ����� �׷���)
		OverSize,			// �ܰ����� �������� �ʰ� ���� ������Ʈ�� ����� Ű���� �������� �� ���ٽ� ����ŷ�� ���� �ܰ����� �׸� 
							// (������Ʈ�� ���α����� �ܰ����� �׷����� ������, �β��� ����� �ܰ����� �׸� �� ����)
	};
	
private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_ResourceManager;
	ID3D11DeviceContext* m_DC;

	Type m_Type;
	float m_Size;			// ���� ������ Ÿ�� �� �ܰ��� �β�

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer_ForDepthMap;			// ���� �ʰ� ���ε��Ͽ� ���
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView_ForDepthMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_DepthMap;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;						// ���� ������Ʈ �н��� ���������� ���� ���
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

	std::unique_ptr<RenderTarget> m_NormalW;		// ���ö� ���� �� �븻 �̱��

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
