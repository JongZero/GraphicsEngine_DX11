#include "TextureLoader.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "DX11Core.h"
#include "MacroDefine.h"
#include "EMath.h"
#include "Mathhelper.h"
#include "ErrChecker.h"

TextureLoader::TextureLoader(std::shared_ptr<DX11Core> dx11Core)
	: m_DX11Core(dx11Core), m_AllTextureCount(0)
{

}

TextureLoader::~TextureLoader()
{
}

ID3D11ShaderResourceView* TextureLoader::GetDiffuseMap(std::wstring textureFileName)
{
	if (m_DiffuseUMap.count(textureFileName) > 0)
	{
		return m_DiffuseUMap[textureFileName].Get();
	}

	return nullptr;
}

ID3D11ShaderResourceView* TextureLoader::GetNormalMap(std::wstring textureFileName)
{
	if (m_NormalUMap.count(textureFileName) > 0)
	{
		return m_NormalUMap[textureFileName].Get();
	}

	return nullptr;
}

ID3D11ShaderResourceView* TextureLoader::GetEmissiveMap(std::wstring textureFileName)
{
	if (m_EmissiveUMap.count(textureFileName) > 0)
	{
		return m_EmissiveUMap[textureFileName].Get();
	}

	return nullptr;
}

ID3D11ShaderResourceView* TextureLoader::GetParticle(std::wstring textureFileName)
{
	if (m_ParticleUMap.count(textureFileName) > 0)
	{
		return m_ParticleUMap[textureFileName].Get();
	}

	return nullptr;
}

ID3D11ShaderResourceView* TextureLoader::GetSkyCubeMap(std::wstring textureFileName)
{
	if (m_SkyCubeUMap.count(textureFileName) > 0)
	{
		return m_SkyCubeUMap[textureFileName].Get();
	}

	return nullptr;
}

ID3D11ShaderResourceView* TextureLoader::GetMetallicMap(std::wstring textureFileName)
{
	if (m_MetallicUMap.count(textureFileName) > 0)
	{
		return m_MetallicUMap[textureFileName].Get();
	}

	return nullptr;
}

ID3D11ShaderResourceView* TextureLoader::GetRoughnessMap(std::wstring textureFileName)
{
	if (m_RoughnessUMap.count(textureFileName) > 0)
	{
		return m_RoughnessUMap[textureFileName].Get();
	}

	return nullptr;
}

ID3D11ShaderResourceView* TextureLoader::GetAOMap(std::wstring textureFileName)
{
	if (m_AOUMap.count(textureFileName) > 0)
	{
		return m_AOUMap[textureFileName].Get();
	}

	return nullptr;
}

void TextureLoader::LoadAllTexturesAndAdd()
{
	CreateTexturesAndAdd(m_FilePathAndFileNameUMap_DiffuseMap, m_DiffuseUMap);
	CreateTexturesAndAdd(m_FilePathAndFileNameUMap_NormalMap, m_NormalUMap);
	CreateTexturesAndAdd(m_FilePathAndFileNameUMap_EmissiveMap, m_EmissiveUMap);
	CreateTexturesAndAdd(m_FilePathAndFileNameUMap_Particle, m_ParticleUMap);
	CreateTexturesAndAdd(m_FilePathAndFileNameUMap_SkyCubeMap, m_SkyCubeUMap);
	CreateTexturesAndAdd(m_FilePathAndFileNameUMap_MetallicMap, m_MetallicUMap);
	CreateTexturesAndAdd(m_FilePathAndFileNameUMap_RoughnessMap, m_RoughnessUMap);
	CreateTexturesAndAdd(m_FilePathAndFileNameUMap_AOMap, m_AOUMap);
}

void TextureLoader::CreateTexturesAndAdd(std::unordered_map<std::wstring, std::wstring>& filePathAndNameMap, std::unordered_map<std::wstring, ComPtr<ID3D11ShaderResourceView>>& textureMap)
{
	std::wstring _fileFilter = L".dds";

	ID3D11Resource* texResource =                 nullptr;
	ID3D11ShaderResourceView* srv = nullptr;

	for (const auto& it : filePathAndNameMap)
	{
		std::wstring fileExt = it.first.substr(it.first.size() - 4, 4);

		// .dds 파일일 경우
		if (fileExt == _fileFilter)
		{
			DirectX::CreateDDSTextureFromFile(m_DX11Core->GetDevice(),
				it.first.c_str(), &texResource, &srv);
		}
		// dds 이외의 파일이면
		else
		{
			DirectX::CreateWICTextureFromFile(m_DX11Core->GetDevice(), m_DX11Core->GetDC(),
				it.first.c_str(), &texResource, &srv);
		}

		if (srv != nullptr)
		{
			// 파일이름과 그에 맞는 비트맵을 넣는다.
			textureMap[it.second] = srv;
		}
	}

	ReleaseCOM(texResource)	// view saves reference
}

ID3D11ShaderResourceView* TextureLoader::CreateRandomTexture1DSRV()
{
	ID3D11Device* device = m_DX11Core->GetDevice();

	// 
	// Create the random data.
	//
	EMath::Vector4 randomValues[1024];

	for (int i = 0; i < 1024; ++i)
	{
		randomValues[i].x = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].y = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].z = MathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].w = MathHelper::RandF(-1.0f, 1.0f);
	}

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = randomValues;
	initData.SysMemPitch = 1024 * sizeof(EMath::Vector4);
	initData.SysMemSlicePitch = 0;

	//
	// Create the texture.
	//
	D3D11_TEXTURE1D_DESC texDesc;
	texDesc.Width = 1024;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.ArraySize = 1;

	ID3D11Texture1D* randomTex = 0;
	HR(device->CreateTexture1D(&texDesc, &initData, &randomTex));

	//
	// Create the resource view.
	//
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;

	ID3D11ShaderResourceView* randomTexSRV = 0;
	HR(device->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV));

	ReleaseCOM(randomTex);

	return randomTexSRV;
}