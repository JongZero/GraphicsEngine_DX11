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
/// view, proj를 받아와서 DepthMap을 그린다.
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

	// 디버그용
	int m_MyNum;

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_DepthMap;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
	D3D11_VIEWPORT m_ViewPort;

	EMath::Matrix m_LightView;		// Light의 View
	EMath::Matrix m_LightProj;		// Light의 Proj

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

	void PreMap();	// 쉐도우맵용 뷰포트 세팅
	void Map(const Shared_ObjectData* objectDataForRender);
	void PostMap(ICore* pCore);						// 이펙트에 DepthMap 세팅, 뷰포트 원상복귀
};

