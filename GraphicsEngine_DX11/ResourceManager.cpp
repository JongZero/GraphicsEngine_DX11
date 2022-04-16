#include "ResourceManager.h"
#include "ObjectMeshData.h"
#include "TextureLoader.h"
#include "MacroDefine.h"
#include "FBXParser.h"
#include "ERJParser.h"
#include "GraphicsEngineMacroDefine.h"
#include "Mesh.h"

#include <filesystem>

ResourceManager::ResourceManager(std::shared_ptr<DX11Core> dx11Core)
	: m_AllResourcesCount(0)
{
	m_TextureLoader = std::make_unique<TextureLoader>(dx11Core);

	m_FBXParser = std::make_unique<FBXParser>();
	m_ERJParser = std::make_unique<ERJParser>();
}

ResourceManager::~ResourceManager()
{
}

ObjectMeshData* ResourceManager::GetObjectMeshData(std::wstring objName)
{
	// �̸��� ���� ������Ʈ �޽� �����Ͱ� �� �ȿ� ������ (������ 0)
	if (m_ObjectMeshDataUMap.count(objName) > 0)
	{
		// ��ȯ���ش�.
		return m_ObjectMeshDataUMap[objName].get();
	}
	else
	{
		return nullptr;
	}
}

void ResourceManager::AddObjectMeshData(std::wstring objName, std::unique_ptr<ObjectMeshData> pObjectMeshData)
{
	m_ObjectMeshDataUMap[objName] = std::move(pObjectMeshData);
}

void ResourceManager::AddUniqueMesh(std::unique_ptr<Mesh> mesh)
{
	m_UniqueMeshVec.push_back(std::move(mesh));
}

void ResourceManager::DeleteObjectMeshData(std::wstring objName)
{
	// �̸��� ���� ������Ʈ �޽� �����Ͱ� �� �ȿ� ������ (������ 0)
	if (m_ObjectMeshDataUMap.count(objName) > 0)
	{
		// �����.
		m_ObjectMeshDataUMap[objName] = nullptr;

		// �ʿ��� ����.
		m_ObjectMeshDataUMap.erase(objName);
	}
}

ID3D11ShaderResourceView* ResourceManager::GetDiffuseMap(std::wstring textureFileName)
{
	return m_TextureLoader->GetDiffuseMap(textureFileName);
}

ID3D11ShaderResourceView* ResourceManager::GetNormalMap(std::wstring textureFileName)
{
	return m_TextureLoader->GetNormalMap(textureFileName);
}

ID3D11ShaderResourceView* ResourceManager::GetEmissiveMap(std::wstring textureFileName)
{
	return m_TextureLoader->GetEmissiveMap(textureFileName);
}

ID3D11ShaderResourceView* ResourceManager::GetParticle(std::wstring textureFileName)
{
	return m_TextureLoader->GetParticle(textureFileName);
}

ID3D11ShaderResourceView* ResourceManager::GetSkyCubeMap(std::wstring textureFileName)
{
	return m_TextureLoader->GetSkyCubeMap(textureFileName);
}

ID3D11ShaderResourceView* ResourceManager::GetMetallicMap(std::wstring textureFileName)
{
	return m_TextureLoader->GetMetallicMap(textureFileName);
}

ID3D11ShaderResourceView* ResourceManager::GetRoughnessMap(std::wstring textureFileName)
{
	return m_TextureLoader->GetRoughnessMap(textureFileName);
}

ID3D11ShaderResourceView* ResourceManager::GetAOMap(std::wstring textureFileName)
{
	return m_TextureLoader->GetAOMap(textureFileName);
}

std::unordered_map<std::wstring, std::wstring>& ResourceManager::GetFilePathAndFileNameUMap()
{
	switch (m_ParserType)
	{
	case ParserType::FBX:
		return m_FBXParser->GetFilePathAndFileNameUMap_FBX();
	case ParserType::ERJ:
		return m_ERJParser->GetFilePathAndFileNameUMap_ERJ();
	}
}

std::vector<ParserData::Mesh*> ResourceManager::GetMeshData(std::wstring fileName)
{
	switch (m_ParserType)
	{
	case ParserType::FBX:
		return m_FBXParser->GetMeshVec(fileName);
	case ParserType::ERJ:
		return m_ERJParser->GetMeshVec(fileName);
	}
}

std::unordered_map<std::wstring, Motion*> ResourceManager::GetObjectMotionData(std::wstring name)
{
	switch (m_ParserType)
	{
	case ParserType::FBX:
		return m_FBXParser->GetObjectMotionData(name);
	case ParserType::ERJ:
		return m_ERJParser->GetObjectMotionData(name);
	}
}

int ResourceManager::LoadAllResourcesCount(std::wstring folderPath, ParserType type)
{
	m_ParserType = type;

	// �ؽ����� ������ �˾Ƴ��� ��ο� ������ �̸��� �����Ѵ�.
	LoadFilePathAndName_Recursive(folderPath + L"/DiffuseMap/", m_TextureLoader->GetFilePathAndFileNameUMap_DiffuseMap());
	LoadFilePathAndName_Recursive(folderPath + L"/NormalMap/", m_TextureLoader->GetFilePathAndFileNameUMap_NormalMap());
	LoadFilePathAndName_Recursive(folderPath + L"/EmissiveMap/", m_TextureLoader->GetFilePathAndFileNameUMap_EmissiveMap());
	LoadFilePathAndName_Recursive(folderPath + L"/Particle/", m_TextureLoader->GetFilePathAndFileNameUMap_Particle());
	LoadFilePathAndName_Recursive(folderPath + L"/SkyCubeMap/", m_TextureLoader->GetFilePathAndFileNameUMap_SkyCubeMap());
	LoadFilePathAndName_Recursive(folderPath + L"/MetallicMap/", m_TextureLoader->GetFilePathAndFileNameUMap_MetalicMap());
	LoadFilePathAndName_Recursive(folderPath + L"/RoughnessMap/", m_TextureLoader->GetFilePathAndFileNameUMap_RoughnessMap());
	LoadFilePathAndName_Recursive(folderPath + L"/AOMap/", m_TextureLoader->GetFilePathAndFileNameUMap_AOMap());

	// ASE ���ϵ�... or FBX ���ϵ�...
	// ASE��� ASE Parser���� FBX��� FBX parser���� ����
	switch (m_ParserType)
	{
	case ParserType::FBX:
		LoadFilePathAndName_Recursive(folderPath + L"/FBX/", m_FBXParser->GetFilePathAndFileNameUMap_FBX());
		break;
	case ParserType::ERJ:
		LoadFilePathAndName_Recursive(folderPath + L"/ERJ/", m_ERJParser->GetFilePathAndFileNameUMap_ERJ());
		break;
	}

	return 0;
}

void ResourceManager::LoadAllResources()
{
	m_TextureLoader->LoadAllTexturesAndAdd();

	switch (m_ParserType)
	{
	case ParserType::FBX:
		m_FBXParser->LoadAllFBXFiles();
		break;
	case ParserType::ERJ:
		m_ERJParser->LoadAllERJFiles();
		break;
	}
}

ID3D11ShaderResourceView* ResourceManager::CreateRandomTexture1DSRV()
{
	return m_TextureLoader->CreateRandomTexture1DSRV();
}

void ResourceManager::LoadFilePathAndName_Recursive(std::wstring folderPath, std::unordered_map<std::wstring, std::wstring>& map)
{
	const std::filesystem::path _folderPath{ folderPath.c_str() };
	std::filesystem::create_directories(_folderPath);

	for (const auto& file : std::filesystem::recursive_directory_iterator{ _folderPath })
	{
		std::wstring filePath = file.path();
		std::wstring fileName = filePath.substr(filePath.rfind(L"/") + 1, filePath.rfind(L"."));

		// Ȯ���� ����
		fileName = fileName.substr(0, fileName.size() - 4);

		// _�� �������  /// **_** ���� �ϴ� emissive�� ����
		if (fileName.find(L"_e") != std::wstring::npos)
		{
			// _e, _n ��� ����
			fileName = fileName.substr(0, fileName.find(L"_e"));
		}

		// file path & file name ����
		map[filePath] = fileName;
	}

	m_AllResourcesCount += map.size();
}
