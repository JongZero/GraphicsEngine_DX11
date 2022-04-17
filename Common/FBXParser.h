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
/// FBX������ �ε��ϰ� �׷��Ƚ� �������� ���� �� �ְ� �ٲ��ִ� �ļ�
/// 2021. 11. 10 ������
/// </summary>
class FBXParser
{
public:
	FBXParser();
	~FBXParser();

private:
	FbxManager* m_pFBXManager;
	unsigned int m_ParsingCount;

	EMath::Matrix m_AxisTransformMatrix;												// �� ��ȯ�� ���� Ʈ������ ��Ʈ����

	std::unordered_map<std::wstring, std::wstring> m_FilePathAndFileNameMap_FBX;		// FBX ������ ��ο� �̸��� ����ִ� ��

	std::unordered_map<std::wstring, std::vector<ParserData::Mesh*>> m_FBXMeshVecUMap;	// n���� FBX ������ �޽� ������ ��

	// �޽� �����͸� �Ľ��ϱ� ���� ������, �ʱ�ȭ �ʿ�
	std::vector<ParserData::Mesh*> m_FBXMeshVec;										// ���� �Ľ����� FBX ������ �޽� ����
	std::map<ParserData::Vertex, unsigned int> m_CheckSplitedVertexMap;					// ���� �Ľ����� FBX�� ���ø��� ���ؽ����� üũ�ϱ� ���� ��

private:
	// �ִϸ��̼� ������
	std::unordered_map<std::wstring, std::unordered_map<std::wstring, Motion*>> m_ObjectMotionUMap;	// �� ���� ������Ʈ�� ��� ����� ����ִ�. 
																									// ù��° ��Ʈ���� ������Ʈ�� �̸�, �ι�° ��Ʈ���� ����� �̸��̴�.
																									// ex) Hero��� ������Ʈ�� Attack, Move ����� ����

	// �ִϸ��̼� �����͸� �Ľ��ϱ� ���� ������, �ʱ�ȭ �ʿ�
	FbxTime::EMode m_FbxTimeMode;
	size_t m_StartFrame;
	size_t m_TotalFrame;
	double m_TicksPerFrame;

	std::unique_ptr<Motion> m_NowMotion;			// ���� �Ľ����� ��� (�ִϸ��̼� �������� ����)
	size_t m_NowBoneIndex;							// ���� �Ľ����� FBX�� �� �ε���

	FbxNode* m_pSkinnedMeshFbxNode;					// ���� �Ľ����� FBX�� Skinned Mesh ��� (���� ���� �а� �Ŀ� Skinned Mesh���� �����͸� �б� ����)
	ParserData::Mesh* m_pSkinnedMesh;				// ���� �Ľ����� FBX�� Skinned Mesh
	std::vector<ParserData::Mesh*> m_BoneVec;		// ���� �Ľ����� FBX�� Bone��, �޽� �ܰ迡�� ����

	// ���ҽ� ������ ���� �����Ǹ� ��Ƴ���
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

	// Normal, UV, Tangent���� ���θ��� ������忡 ���� �о����
	EMath::Vector3 ReadNormal(FbxMesh* fbxMesh, int vertexIndex, int vertexCount);
	EMath::Vector2 ReadUV(FbxMesh* fbxMesh, int vertexIndex, int vertexCount);
	EMath::Vector3 ReadTangent(FbxMesh* fbxMesh, int vertexIndex, int vertexCount);
};
