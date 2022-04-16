#pragma once
#include <string>
#include <map>
#include <unordered_map>
#include <d3d11.h>
#include <memory>
#include "ParserData.h"

enum class ParserType;
class DX11Core;
class TextureLoader;
class FBXParser;
class ERJParser;
struct ObjectMeshData;
struct Motion;
struct Mesh;

/// <summary>
/// Mesh Creator���� ������ �޽� ������ �� DX���۵��� �̰����� �����������Ѵ�.
/// �̿ܿ� �ؽ���,... ���� ���ҽ����� �����Ѵ�.
/// 2021. 10. 27 ������
/// </summary>
class ResourceManager
{
public:
	ResourceManager(std::shared_ptr<DX11Core> dx11Core);
	~ResourceManager();

private:
	std::unique_ptr<TextureLoader> m_TextureLoader;
	std::unique_ptr<FBXParser> m_FBXParser;
	std::unique_ptr<ERJParser> m_ERJParser;

private:
	ParserType m_ParserType;
	size_t m_AllResourcesCount;

	std::unordered_map<std::wstring, std::unique_ptr<ObjectMeshData>> m_ObjectMeshDataUMap;
	std::vector<std::unique_ptr<Mesh>> m_UniqueMeshVec;

public:
	ObjectMeshData* GetObjectMeshData(std::wstring objName);
	void AddObjectMeshData(std::wstring objName, std::unique_ptr<ObjectMeshData> pObjectMeshData);
	void AddUniqueMesh(std::unique_ptr<Mesh> mesh);
	void DeleteObjectMeshData(std::wstring objName);

	ID3D11ShaderResourceView* GetDiffuseMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetNormalMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetEmissiveMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetParticle(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetSkyCubeMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetMetallicMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetRoughnessMap(std::wstring textureFileName);
	ID3D11ShaderResourceView* GetAOMap(std::wstring textureFileName);

	/// FBX �ļ� ������
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap();
	std::vector<ParserData::Mesh*> GetMeshData(std::wstring fileName);
	std::unordered_map<std::wstring, Motion*> GetObjectMotionData(std::wstring name);	// �ִϸ��̼� ������

public:
	/// �ε����� ���ؼ� �켱 ���ҽ� ������ ������ �ľ��ϰ� ��θ� �����ص� �Ŀ�
	/// ���� ������ ���� �ε����� ��� �� �׷��Ƚ� �������� ����� �������� ���ҽ��� ����鼭
	/// �ε����� �ε� ��(�����)�� �����Ѵ�.
	int LoadAllResourcesCount(std::wstring folderPath, ParserType type);
	void LoadAllResources();
	ID3D11ShaderResourceView* CreateRandomTexture1DSRV();

private:
	void LoadFilePathAndName_Recursive(std::wstring folderPath, std::unordered_map<std::wstring, std::wstring>& map);
};
