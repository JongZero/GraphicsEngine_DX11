#pragma once
#include <memory>
#include <string>
#include <unordered_map>

class ResourceManager;
struct Mesh;
struct ObjectMeshData;
struct Motion;

/// <summary>
/// ���ӿ������κ��� ������Ʈ�� �̸��� �޾ƿ� ���ҽ� �Ŵ����� �ش�Ǵ� �޽�, �ִϸ��̼��� ������ �ʱ�ȭ�ϰ� �ִϸ��̼��� ������Ʈ ��Ų��.
/// </summary>
class Animator
{
public:
	Animator(std::shared_ptr<ResourceManager> rm);
	~Animator();

private:
	std::shared_ptr<ResourceManager> m_ResourceManager;		// ���ҽ� �Ŵ����κ��� �޽� �����Ϳ� �ִϸ��̼� �����͸� �����;��Ѵ�.

public:
	void InitializeAnimation(const std::wstring& objName, bool& isCompleted);
	void UpdateAnimation(const std::wstring& objName, const std::wstring& nowMotionName, float dTime, float speed);

private:
	// �޽��� �θ� ����
	void SetParent(std::unordered_map<std::wstring, Motion*>& motionDataUMap, ObjectMeshData* objMeshData);		
	// �޽��� �θ� �ִ���
	bool IsParentExist(Mesh* mesh);		
	// ��ǵ��� �ִϸ��̼� �ʱ� ������ ���ش�.
	void InitializeAnimation(std::unordered_map<std::wstring, Motion*>& motionDataUMap, ObjectMeshData* objMeshData);	
	void SetBoneAndSkinnedMesh(std::unordered_map<std::wstring, Motion*>& motionDataUMap, ObjectMeshData* objMeshData);
	void ResetMotion(Motion* motion);		// *_* Ȯ���ʿ�
};