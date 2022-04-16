#pragma once
#include <memory>
#include <string>
#include <unordered_map>

class ResourceManager;
struct Mesh;
struct ObjectMeshData;
struct Motion;

/// <summary>
/// 게임엔진으로부터 오브젝트의 이름을 받아와 리소스 매니저에 해당되는 메쉬, 애니메이션이 있으면 초기화하고 애니메이션을 업데이트 시킨다.
/// </summary>
class Animator
{
public:
	Animator(std::shared_ptr<ResourceManager> rm);
	~Animator();

private:
	std::shared_ptr<ResourceManager> m_ResourceManager;		// 리소스 매니저로부터 메쉬 데이터와 애니메이션 데이터를 가져와야한다.

public:
	void InitializeAnimation(const std::wstring& objName, bool& isCompleted);
	void UpdateAnimation(const std::wstring& objName, const std::wstring& nowMotionName, float dTime, float speed);

private:
	// 메쉬의 부모를 세팅
	void SetParent(std::unordered_map<std::wstring, Motion*>& motionDataUMap, ObjectMeshData* objMeshData);		
	// 메쉬가 부모가 있는지
	bool IsParentExist(Mesh* mesh);		
	// 모션들의 애니메이션 초기 설정을 해준다.
	void InitializeAnimation(std::unordered_map<std::wstring, Motion*>& motionDataUMap, ObjectMeshData* objMeshData);	
	void SetBoneAndSkinnedMesh(std::unordered_map<std::wstring, Motion*>& motionDataUMap, ObjectMeshData* objMeshData);
	void ResetMotion(Motion* motion);		// *_* 확인필요
};