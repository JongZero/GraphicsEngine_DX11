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
/// Mesh Creator에서 생성된 메쉬 데이터 및 DX버퍼들을 이곳에서 관리·보관한다.
/// 이외에 텍스쳐,... 같은 리소스들을 보관한다.
/// 2021. 10. 27 정종영
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

	/// FBX 파서 데이터
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap();
	std::vector<ParserData::Mesh*> GetMeshData(std::wstring fileName);
	std::unordered_map<std::wstring, Motion*> GetObjectMotionData(std::wstring name);	// 애니메이션 데이터

public:
	/// 로딩씬을 위해서 우선 리소스 파일의 개수를 파악하고 경로만 저장해둔 후에
	/// 개수 정보를 토대로 로딩씬을 띄운 후 그래픽스 엔진에서 사용할 실직적인 리소스를 만들면서
	/// 로딩씬의 로딩 바(진행률)를 갱신한다.
	int LoadAllResourcesCount(std::wstring folderPath, ParserType type);
	void LoadAllResources();
	ID3D11ShaderResourceView* CreateRandomTexture1DSRV();

private:
	void LoadFilePathAndName_Recursive(std::wstring folderPath, std::unordered_map<std::wstring, std::wstring>& map);
};
