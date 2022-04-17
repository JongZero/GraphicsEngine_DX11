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
/// ���� ������Ʈ���� ��ġ�ǰ� �ش� ������Ʈ���� ������Ʈ�� ������ �����ؼ� ������Ʈ�Ѵ�.
/// 
/// 2021.11.02 ������
/// </summary>
class Scene :public IEvent
{
private:
	friend class GameObject;
public:
	std::wstring m_Name;								//�� �̸�
	float m_Aspect_Ratio;								//���� ��Ⱦ��
	Camera* m_MainCamera;								//���� ���� ī�޶�
	Light* m_DirectionalLight;							//���� �𷺼ų� ����Ʈ

	bool m_IsNewStart;									//������Ʈ �� ��ŸƮ�Լ� ������ �ʿ����� ����

	Shared_RenderingData* m_DataForRender;						//���� �׸��� ���� ������
	std::vector<GameObject*> m_GameObjects;				//���� �����ִ� ���� ������Ʈ��
	std::multimap<eComponentOrder, Component*> m_OrderOfExecution;	//������Ʈ���� ���� ������ ���ؼ� �����Ѵ�.

public:
	Scene(std::wstring name);
	Scene(std::wstring name, float aspect_ratio);
	//����Ƽ ������ 
	Scene(std::wstring name, float aspect_ratio, bool isunity);

	virtual ~Scene();
public:
	//���� ������Ʈ �̸� �ߺ� üũ
	std::wstring CheckOverlappedObjectName(std::wstring name);

	//���� ī�޶��� �ʱ� ����
	void SetMainCamera();

	//�𷺼ų� ����Ʈ �ʱ� ����
	void SetDirectionalLight();

	//�� �̸� ��ȯ
	std::wstring GetName() const { return m_Name; };

	//���� ������ ���� ������Ʈ �߰�
	void AddGameObject(GameObject* gameObject);

	//������ ������Ʈ�� ����
	void CreateGameObject(std::wstring name);

	//������ ������Ʈ�� ����
	void RemoveGameObject(std::wstring name);

	//������ ������Ʈ�� ��ȯ
	GameObject* GetGameObject(std::wstring name);

	//���� �׸��� ���� �����͸� ��ȯ
	Shared_RenderingData* GetDataForRender() { return m_DataForRender; };

	//������Ʈ �� ������ ������Ʈ���� start()�Լ� ȣ���
	void NewStart();


	//������Ʈ ���� ���� �ο�
	template <typename T>
	void AddOrderOfExecution(T* component);

	//������Ʈ�� ������ ���
	template <typename T>
	void RemoveOrderOfExecution(T* component);

	//���ӿ�����Ʈ ������ ���
	template <typename T>
	void RemoveOrderOfExecution(GameObject* gameObject);


	/// MFC �� �Լ�
public:
	//�� ������ ������Ʈ ��
	int GetGameObjectSize();
	//
	std::wstring GetGameObjectName(int index);
	//���� ������ ��ŷ�� ������Ʈ�� �̸� ��ȯ
	std::wstring GetPickedObject();
	//�̸��� �޾� ��ŷ�� ������Ʈ�� �����Ѵ�.
	void SetPickedObject(std::wstring name);

	//IEVENT���� ��ӹ��� �Լ�
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
	//������Ʈ�� ������ ���ӿ�����Ʈ�� ������ �μ��� ���� ������Ʈ�� ���Ѵ�.
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

