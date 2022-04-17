#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#pragma once
#include <typeinfo>
#include <vector>

#include "Object.h"
#include "Transform.h"
#include "Scene.h"

//전방선언
class Component;
//class Scene;
class Collider;
class Camera;
class Light;
class Animator;
class MeshRenderer;
class MeshFilter;
class Script;

/// <summary>
/// 컴포넌트를 가질 수 있는 객체이다.
/// 2021.10.19 강주현
/// </summary>
class GameObject :public Object
{
private:
	friend class Scene;

public:
	//std::wstring m_name() Object
	bool m_ActiveInHierarchy;	//게임오브젝트가 씬에서 활성 상태인지 여부 
	bool  m_ActiveSelf;			//게임오브젝트의 로컬 활성 상태 
//	bool isStatic;				
	Transform* m_Transform;		//게임오브젝에 연결된 Transform
	std::wstring m_Tag;			//게임오브젝트의 tag
	Scene* m_Scene;				//게임오브젝트가 속한 씬

private:
	std::vector<Component*> m_Components;	// 컴포넌트들

public:
	//GameObject();
	GameObject(std::wstring name);
	GameObject(std::wstring name, Scene* scene);
	virtual ~GameObject();

protected:
	//복사생성자 대응
	GameObject(GameObject& rhs);
public:

	// 게임오브젝트의 이름을 설정
	void SetName(std::wstring name) { m_Name = name; };
	// 게임오브젝트의 태그 설정
	void SetTag(std::wstring tag) { m_Tag = tag; };
	//게임오브젝트가 속한 씬을 설정
	void SetScene(Scene* scene) { m_Scene = scene; };

	//컴포넌트 개수를 반환
	int GetComponentSize();


public:
	//오브젝트에 컴포넌트를 추가
	template <typename T>
	T AddComponent(T);
	//오브젝트로부터 컴포넌트를 반환
	template <typename T>
	T* GetComponent();


	/// 현재 미사용
	template <typename T>
	T GetComponent2();

	//오브젝트로부터 컴포넌트를 제거
	template <typename T>
	void RemoveComponent();

	//씬의 컴포넌트 순서배열에 추가
	void AddOrder(Component* component);

	//씬의 컴포넌트 순서배열에서 제거
	void RemoveOrder(Component* component);
public:
	//게임 오브젝트의 태그를 비교한다.
	bool CompareTag(std::wstring tag);

	//게임 오브젝트를  활성/비활성화 시킨다.
	void SetActive(bool value);

	//게임 오브젝트를  활성화 여부 확인
	bool GetActive()const;

	//인덱스를 입력받아서 컴포넌트 반환-> for문 이용 컴포넌트 돌기용으로 사용 
	Component* GetComponentIndex(int index);

	//오브젝트의 컴포넌트 배열을 반환
	std::vector<Component*> GetComponents() { return m_Components; };

	//Object에서 상속된 함수
public:
	void Destroy() override {};
	void DontDestroyOnLoad() override {};
	//void FindObjectOfType() override {};
	//void FindObjectsOfType() override {};
	void Instantiate() override {};

	//IEVENT에서 상속받은 함수
public:
	void	Awake() override {};
	void	Start() override {};
	void	FixedUpdate() override {};
	void	InputEvents() override {};
	void	Update(float dtime) override {};
	void	LateUpdate() override {};
	void	PreRender() override {};
	void	Render() override {};
	void	PostRender() override {};
};

template <typename T>
T GameObject::AddComponent(T component)
{
	
	//중복이 아닐결우
	if (GetComponent<T>() == nullptr)
	{
		Component* _co = dynamic_cast<Component*>(component);
		_co->m_GameObject = this;
		_co->m_Tag = this->m_Tag;
		_co->m_Transform = this->m_Transform;


		m_Components.push_back(_co);

		if (m_Scene)
		{
			AddOrder(_co);
		}
		return component;
	}
	else
	{
		return nullptr;
	}
}

template <typename T>
T* GameObject::GetComponent()
{
	for (Component* _co : m_Components)
	{
		const type_info& _info = typeid(*_co);
		const type_info& _infoT = typeid(T);

		size_t _hash1 = _info.hash_code();
		size_t _hash2 = _infoT.hash_code();

		//string name1 = _info.name();
		//string name2 = _infoT.name();

		if (typeid(*_co) == typeid(T))
		{

			//why error dynamic? 
			return reinterpret_cast<T*>(_co);
		}
	}

	return nullptr;
}

template <typename T>
T GameObject::GetComponent2()
{
	for (Component* co : m_Components)
	{
		const type_info& _info = typeid(*co);
		const type_info& _infoT = typeid(T);

		std::string name1 = _info.name();
		std::string name2 = _infoT.name();

		//if (typeid(*co) == typeid(T))	// *T를 사용할 수 없음.

		// 다이내믹 캐스트는 비용 발생 가능성이 있음.
		// (하지만 하나의 엔티티에 컴포넌트가 몇 개나 있을까)
		T _co = dynamic_cast<T>(co);
		if (NULL != _co)
		{
			return _co;
		}
	}

	return nullptr;
}

template<typename T>
void GameObject::RemoveComponent()
{
	int _index = 0;
	for (Component* _co : m_Components)
	{
		const type_info& _info = typeid(*_co);
		const type_info& _infoT = typeid(T);

		size_t _hash1 = _info.hash_code();
		size_t _hash2 = _infoT.hash_code();


		if (typeid(*_co) == typeid(T))
		{
			m_Components.erase(m_Components.begin() + _index);
			RemoveOrder(_co);
			break;
		}

		_index++;
	}

}

#endif
