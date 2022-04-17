#ifndef SCENE_H
#define SCENE_H
#pragma once

#include <map>
#include "GameObject.h"
#include "Component.h"

#include "../EngineCore/Camera.h"
#include "../EngineCore/Light.h"
#include "../EngineCore/MeshRenderer.h"
#include "../EngineCore/MeshFilter.h"
#include "../EngineCore/Animator.h"
#include "../EngineCore/Script.h"


// #include "DataForRender.h"

class GameObject;
class Component;
class Transform;
class Collider;
class Camera;
class Light;
class MeshRenderer;
class Animator;
class Script;
struct Shared_RenderingData;

/// <summary>
/// 게임 오브젝트들이 배치되고 해당 오브젝트들의 컴포넌트를 별도로 보관해서 업데이트한다.
/// 
/// 2021.11.02 강주현
/// </summary>
class Scene :public IEvent
{
private:
	friend class GameObject;
public:
	std::wstring m_Name;								//씬 이름
	float m_Aspect_Ratio;								//씬의 종횡비
	Camera* m_MainCamera;								//씬의 메인 카메라
	Light* m_DirectionalLight;							//씬의 디렉셔널 라이트

	bool m_IsNewStart;									//컴포넌트 중 스타트함수 실행이 필요한지 여부

	Shared_RenderingData* m_DataForRender;						//씬을 그리기 위한 데이터
	std::vector<GameObject*> m_GameObjects;				//씬에 속해있는 게임 오브젝트들
	std::multimap<eComponentOrder, Component*> m_OrderOfExecution;	//컴포넌트간의 실행 순서을 정해서 저장한다.

public:
	Scene(std::wstring name);
	Scene(std::wstring name, float aspect_ratio);
	//유니티 씬전용 
	Scene(std::wstring name, float aspect_ratio, bool isunity);

	virtual ~Scene();
public:
	//게임 오브젝트 이름 중복 체크
	std::wstring CheckOverlappedObjectName(std::wstring name);

	//메인 카메라의 초기 세팅
	void SetMainCamera();

	//디렉셔널 라이트 초기 세팅
	void SetDirectionalLight();

	//씬 이름 반환
	std::wstring GetName() const { return m_Name; };

	//씬에 생성된 게임 오브젝트 추가
	void AddGameObject(GameObject* gameObject);

	//씬에서 오브젝트를 생성
	void CreateGameObject(std::wstring name);

	//씬에서 오브젝트를 삭제
	void RemoveGameObject(std::wstring name);

	//씬에서 오브젝트를 반환
	GameObject* GetGameObject(std::wstring name);

	//씬을 그리기 위한 데이터를 반환
	Shared_RenderingData* GetDataForRender() { return m_DataForRender; };

	//업데이트 중 생성된 컴포넌트들의 start()함수 호출용
	void NewStart();


	//컴포넌트 실행 순서 부여
	template <typename T>
	void AddOrderOfExecution(T* component);

	//컴포넌트만 삭제시 사용
	template <typename T>
	void RemoveOrderOfExecution(T* component);

	//게임오브젝트 삭제시 사용
	template <typename T>
	void RemoveOrderOfExecution(GameObject* gameObject);


	/// MFC 용 함수
public:
	//씬 내부의 오브젝트 수
	int GetGameObjectSize();
	//
	std::wstring GetGameObjectName(int index);
	//현재 씬에서 피킹된 오브젝트의 이름 반환
	std::wstring GetPickedObject();
	//이름을 받아 피킹된 오브젝트를 설정한다.
	void SetPickedObject(std::wstring name);

	//IEVENT에서 상속받은 함수
public:
	void	Awake() override;
	void	Start() override;
	void	FixedUpdate() override;
	void	InputEvents() override;
	void	Update(float dtime) override;
	void	LateUpdate() override;
	void	PreRender() override;
	void	Render() override;
	void	PostRender() override;
};

template<typename T>
inline void Scene::AddOrderOfExecution(T* component)
{
	m_OrderOfExecution.insert(std::pair<eComponentOrder, T*>(component->m_Order, component));
}

template<typename T>
inline void Scene::RemoveOrderOfExecution(T* component)
{
	//컴포넌트를 보유한 게임오브젝트를 가져와 인수로 받은 컴포넌트와 비교한다.
//	GameObject* gameobject = component->m_GameObject;

	std::multimap<eComponentOrder, Component*>::iterator _iter;
	for (_iter = m_OrderOfExecution.begin(); _iter != m_OrderOfExecution.end(); ++_iter)
	{
		if (_iter->second == dynamic_cast<T*>(component))
		{
			m_OrderOfExecution.erase(_iter);
			break;
		}
	}
}
#endif

