#pragma once
#include <unordered_map>
#include <memory>

#include "ParserData.h"
#include "MeshAnimationData.h"
#include "ERJConverter.h"

class ERJParser
{
public:
	ERJParser();
	~ERJParser();

private:
	std::unique_ptr<ERJConverter> m_ERJConverter;

private:
	std::unordered_map<std::wstring, std::wstring> m_FilePathAndFileNameMap_ERJ;		// FBX 파일의 경로와 이름을 들고있는 맵
	std::unordered_map<std::wstring, std::vector<ParserData::Mesh*>> m_MeshVecUMap;		// n개의 FBX 파일의 메쉬 데이터 맵

	std::unordered_map<std::wstring, std::unordered_map<std::wstring, Motion*>> m_ObjectMotionUMap;	// 한 개의 오브젝트의 모든 모션이 들어있다. 

	// 리소스 해제를 위해 소유권만 모아놓음
	std::vector<std::unique_ptr<ParserData::Mesh>> m_UniqueMeshVec;
	std::vector<std::unique_ptr<Motion>> m_UniqueMotionVec;
	std::vector<std::unique_ptr<MeshAnimationData>> m_UniqueAnimDataVec;

public:
	std::unordered_map<std::wstring, std::wstring>& GetFilePathAndFileNameUMap_ERJ() { return m_FilePathAndFileNameMap_ERJ; }
	std::vector<ParserData::Mesh*> GetMeshVec(std::wstring fileName);
	std::unordered_map<std::wstring, Motion*> GetObjectMotionData(std::wstring name);

public:
	void LoadAllERJFiles(bool isPrint = false);

private:
	void LoadFile(std::pair<std::wstring, std::wstring> pathAndName);
};
