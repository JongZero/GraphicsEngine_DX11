#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#pragma once
#include <typeinfo>
#include <vector>

#include "Object.h"
#include "Transform.h"
#include "Scene.h"

//���漱��
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
/// ������Ʈ�� ���� �� �ִ� ��ü�̴�.
/// 2021.10.19 ������
/// </summary>
class GameObject :public Object
{
private:
	friend class Scene;

public:
	//std::wstring m_name() Object
	bool m_ActiveInHierarchy;	//���ӿ�����Ʈ�� ������ Ȱ�� �������� ���� 
	bool  m_ActiveSelf;			//���ӿ�����Ʈ�� ���� Ȱ�� ���� 
//	bool isStatic;				
	Transform* m_Transform;		//���ӿ������� ����� Transform
	std::wstring m_Tag;			//���ӿ�����Ʈ�� tag
	Scene* m_Scene;				//���ӿ�����Ʈ�� ���� ��

private:
	std::vector<Component*> m_Components;	// ������Ʈ��

public:
	//GameObject();
	GameObject(std::wstring name);
	GameObject(std::wstring name, Scene* scene);
	virtual ~GameObject();

protected:
	//��������� ����
	GameObject(GameObject& rhs);
public:

	// ���ӿ�����Ʈ�� �̸��� ����
	void SetName(std::wstring name) { m_Name = name; };
	// ���ӿ�����Ʈ�� �±� ����
	void SetTag(std::wstring tag) { m_Tag = tag; };
	//���ӿ�����Ʈ�� ���� ���� ����
	void SetScene(Scene* scene) { m_Scene = scene; };

	//������Ʈ ������ ��ȯ
	int GetComponentSize();


public:
	//������Ʈ�� ������Ʈ�� �߰�
	template <typename T>
	T AddComponent(T);
	//������Ʈ�κ��� ������Ʈ�� ��ȯ
	template <typename T>
	T* GetComponent();


	/// ���� �̻��
	template <typename T>
	T GetComponent2();

	//������Ʈ�κ��� ������Ʈ�� ����
	template <typename T>
	void RemoveComponent();

	//���� ������Ʈ �����迭�� �߰�
	void AddOrder(Component* component);

	//���� ������Ʈ �����迭���� ����
	void RemoveOrder(Component* component);
public:
	//���� ������Ʈ�� �±׸� ���Ѵ�.
	bool CompareTag(std::wstring tag);

	//���� ������Ʈ��  Ȱ��/��Ȱ��ȭ ��Ų��.
	void SetActive(bool value);

	//���� ������Ʈ��  Ȱ��ȭ ���� Ȯ��
	bool GetActive()const;

	//�ε����� �Է¹޾Ƽ� ������Ʈ ��ȯ-> for�� �̿� ������Ʈ ��������� ��� 
	Component* GetComponentIndex(int index);

	//������Ʈ�� ������Ʈ �迭�� ��ȯ
	std::vector<Component*> GetComponents() { return m_Components; };

	//Object���� ��ӵ� �Լ�
public:
	void Destroy() override {};
	void DontDestroyOnLoad() override {};
	//void FindObjectOfType() override {};
	//void FindObjectsOfType() override {};
	void Instantiate() override {};

	//IEVENT���� ��ӹ��� �Լ�
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
	
	//�ߺ��� �ƴҰ��
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

		//if (typeid(*co) == typeid(T))	// *T�� ����� �� ����.

		// ���̳��� ĳ��Ʈ�� ��� �߻� ���ɼ��� ����.
		// (������ �ϳ��� ��ƼƼ�� ������Ʈ�� �� ���� ������)
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
