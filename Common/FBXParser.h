#pragma once
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include "fbxsdk.h"

#include "ParserData.h"

struct Motion;
struct MeshAnimationData;

/// <summary>
/// FBX파일을 로드하고 그래픽스 엔진에서 읽을 수 있게 바꿔주는 파서
/// 2021. 11. 10 정종영
/// </summary>
class FBXParser
{
public:
	FBXParser();
	~FBXParser();

private:
	FbxManager* m_pFBXManager;
	unsigned int m_ParsingCount;

	EMath::Matrix m_AxisTransformMatrix;												// 축 변환을 위한 트랜스폼 매트릭스

	std::unordered_map<std::wstring, std::wstring> m_FilePathAndFileNameMap_FBX;		// FBX 파일의 경로와 이름을 들고있는 맵

	std::unordered_map<std::wstring, std::vector<ParserData::Mesh*>> m_FBXMeshVecUMap;	// n개의 FBX 파일의 메쉬 데이터 맵

	// 메쉬 데이터를 파싱하기 위한 변수들, 초기화 필요
	std::vector<ParserData::Mesh*> m_FBXMeshVec;										// 현재 파싱중인 FBX 파일의 메쉬 벡터
	std::map<ParserData::Vertex, unsigned int> m_CheckSplitedVertexMap;					// 현재 파싱중인 FBX의 스플릿된 버텍스들을 체크하기 위한 맵

private:
	// 애니메이션 데이터
	std::unordered_map<std::wstring, std::unordered_map<std::wstring, Motion*>> m_ObjectMotionUMap;	// 한 개의 오브젝트의 모든 모션이 들어있다. 
																									// 첫번째 스트링은 오브젝트의 이름, 두번째 스트링은 모션의 이름이다.
																									// ex) Hero라는 오브젝트에 Attack, Move 모션이 존재

	// 애니메이션 데이터를 파싱하기 위한 변수들, 초기화 필요
	FbxTime::EMode m_FbxTimeMode;
	size_t m_StartFrame;
	size_t m_TotalFrame;
	double m_TicksPerFrame;

	std::unique_ptr<Motion> m_NowMotion;			// 현재 파싱중인 모션 (애니메이션 데이터의 묶음)
	size_t m_NowBoneIndex;							// 현재 파싱중인 FBX의 본 인덱스

	FbxNode* m_pSkinnedMeshFbxNode;					// 현재 파싱중인 FBX의 Skinned Mesh 노드 (본을 먼저 읽고 후에 Skinned Mesh에서 데이터를 읽기 위함)
	ParserData::Mesh* m_pSkinnedMesh;				// 현재 파싱중인 FBX의 Skinned Mesh
	std::vector<ParserData::Mesh*> m_BoneVec;		// 현재 파싱중인 FBX의 Bone들, 메쉬 단계에서 생성

	// 리소스 해제를 위해 소유권만 모아놓음
	std::vector<std::unique_ptr<ParserData::Mesh>> m_UniqueMeshVec;
	std::vector<std::unique_ptr<Motion>> m_UniqueMotionVec;
	std::vector<std::unique_ptr<MeshAnimationData>> m_UniqueAnimDataVec;

public:
	const unsigned int& GetParsingCount() { return m_ParsingCount; }
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap_FBX() { return m_FilePathAndFileNameMap_FBX; }
	std::vector<ParserData::Mesh*> GetMeshVec(std::wstring fileName);
	std::unordered_map<std::wstring, Motion*> GetObjectMotionData(std::wstring name);

public:
	void LoadAllFBXFiles(bool isPrint = false);

private:
	void Initialize();
	FbxScene* LoadScene(const char* fileName);
	void ParseFbxScene(FbxScene* scene, std::wstring fileName);

	void CreateMesh_And_LoadNodeData(FbxScene* scene, FbxNode* node);
	void CreateVertex_And_LoadVertexPos(FbxNode* node, ParserData::Mesh* mesh);
	void CreateVertex_And_LoadVertexPos_Animation(FbxNode* node, ParserData::Mesh* mesh);
	void LoadVertexOthers(FbxNode* node, ParserData::Mesh* mesh);
	void SplitVertex(ParserData::Mesh* mesh, ParserData::Vertex& vertex);

	void CheckNegativeScale(ParserData::Mesh* mesh);
	void DownScale(ParserData::Mesh* mesh);

	void LoadSkinnedMeshData(FbxScene* scene);
	void LoadBoneWeights(FbxNode* node, ParserData::Mesh* mesh);
	void LoadAnimationData(FbxScene* scene, FbxNode* node);

	// Normal, UV, Tangent등을 맵핑모드와 참조모드에 따라 읽어들임
	EMath::Vector3 ReadNormal(FbxMesh* fbxMesh, int vertexIndex, int vertexCount);
	EMath::Vector2 ReadUV(FbxMesh* fbxMesh, int vertexIndex, int vertexCount);
	EMath::Vector3 ReadTangent(FbxMesh* fbxMesh, int vertexIndex, int vertexCount);
};
