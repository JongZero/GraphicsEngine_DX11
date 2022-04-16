#pragma once
#include <unordered_map>
#include <d3d11.h>
#include <memory>

// For ComPtr
#include <wrl.h>
using namespace Microsoft::WRL;

class DX11Core;

/// <summary>
/// �ؽ��ĸ� �ε��ϰ� �����ϴ� Ŭ����
/// 2021. 11. 04
/// </summary>
class TextureLoader
{
public:
	TextureLoader(std::shared_ptr<DX11Core> m_DX11Core);
	~TextureLoader();
	
private:
	std::shared_ptr<DX11Core> m_DX11Core;

private:
	// ������ ��� & �̸�
	std::unordered_map<std::wstring, std::wstring> m_FilePathAndFileNameUMap_DiffuseMap;
	std::unordered_map<std::wstring, std::wstring> m_FilePathAndFileNameUMap_NormalMap;
	std::unordered_map<std::wstring, std::wstring> m_FilePathAndFileNameUMap_EmissiveMap;
	std::unordered_map<std::wstring, std::wstring> m_FilePathAndFileNameUMap_Particle;
	std::unordered_map<std::wstring, std::wstring> m_FilePathAndFileNameUMap_SkyCubeMap;
	std::unordered_map<std::wstring, std::wstring> m_FilePathAndFileNameUMap_MetallicMap;
	std::unordered_map<std::wstring, std::wstring> m_FilePathAndFileNameUMap_RoughnessMap;
	std::unordered_map<std::wstring, std::wstring> m_FilePathAndFileNameUMap_AOMap;
	
	// ������ �� �ؽ��� ���� (SRV)
	std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>> m_DiffuseUMap;
	std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>> m_NormalUMap;
	std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>> m_EmissiveUMap;
	std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>> m_ParticleUMap;
	std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>> m_SkyCubeUMap;
	std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>> m_MetallicUMap;
	std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>> m_RoughnessUMap;
	std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>> m_AOUMap;

	// �ؽ��� ������ �� ����
	int m_AllTextureCount;

public:
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap_DiffuseMap() { return m_FilePathAndFileNameUMap_DiffuseMap; }
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap_NormalMap() { return m_FilePathAndFileNameUMap_NormalMap; }
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap_EmissiveMap() { return m_FilePathAndFileNameUMap_EmissiveMap; }
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap_Particle() { return m_FilePathAndFileNameUMap_Particle; }
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap_SkyCubeMap() { return m_FilePathAndFileNameUMap_SkyCubeMap; }
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap_MetalicMap() { return m_FilePathAndFileNameUMap_MetallicMap; }
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap_RoughnessMap() { return m_FilePathAndFileNameUMap_RoughnessMap; }
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap_AOMap() { return m_FilePathAndFileNameUMap_AOMap; }

	ID3D11ShaderResourceView* GetDiffuseMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetNormalMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetEmissiveMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetParticle(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetSkyCubeMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetMetallicMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetRoughnessMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetAOMap(std::wstring textureFileName);

	int GetAllTexturesNum() { return m_AllTextureCount; }

public:
	void LoadAllTexturesAndAdd();
	ID3D11ShaderResourceView* CreateRandomTexture1DSRV();
	
private:
	void CreateTexturesAndAdd(std::unordered_map<std::wstring, std::wstring>& filePathAndNameMap, std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>>& textureMap);
};
