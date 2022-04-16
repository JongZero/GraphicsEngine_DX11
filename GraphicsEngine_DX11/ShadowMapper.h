#pragma once
#include <string>
#include <vector>
#include <d3d11.h>
#include <memory>
#include <wrl.h>

#include "Shared_RenderingData.h"

class DX11Core;
class ResourceManager;
class ICore;

/// <summary>
/// view, proj�� �޾ƿͼ� DepthMap�� �׸���.
/// </summary>
class ShadowMapper
{
public:
	ShadowMapper(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm, int width, int height);
	virtual ~ShadowMapper();

public:
	bool IsActive;

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_ResourceManager;

	std::wstring m_Name;
	int m_Width;
	int m_Height;

	// ����׿�
	int m_MyNum;

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_DepthMap;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
	D3D11_VIEWPORT m_ViewPort;

	EMath::Matrix m_LightView;		// Light�� View
	EMath::Matrix m_LightProj;		// Light�� Proj

private:
	EMath::Matrix m_ShadowTransform;

public:
	EMath::Matrix& GetShadowTransform() { return m_ShadowTransform; }

public:
	ID3D11ShaderResourceView* GetDepthMap() { return m_DepthMap.Get(); }

public:
	void Initialize(int width, int height);
	void StartMap(Shared_RenderingData* dataForRender, ICore* pCore);
	void DrawDebugScreen();

private:
	void BuildShadowTransform(DirectionalLightInfo* mainDirInfo);

	void PreMap();	// ������ʿ� ����Ʈ ����
	void Map(const Shared_ObjectData* objectDataForRender);
	void PostMap(ICore* pCore);						// ����Ʈ�� DepthMap ����, ����Ʈ ���󺹱�
};

