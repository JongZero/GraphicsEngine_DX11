#ifndef TRANSFORM_H
#define TRANSFORM_H
#pragma once
#include "Component.h"

using namespace EMath;

//���� ������ ���� ������ �����Ѵ�.
enum class Space
{
	World = 0, Self
};

/// <summary>
/// ������Ʈ���� ���̾��Ű ������ ���ȴ�.
/// math �� Utility.lib�� ����Ѵ�.
/// 2021.10.19 ������
/// </summary>
class Transform : public Component
{

public:
	Vector3 m_Position;					//���ÿ����� ��ġ
	Quaternion m_Rotation;				//���� ���������� ��ȯ�� ȸ��
	Vector3 m_Scale;					//���ÿ����� ũ��

	Matrix m_WorldMatrix;				//���� ���

	Transform* m_Root;					//��ȯ�� �ֻ��� �θ�	
	Transform* m_Parent;				//��ȯ�� �θ�
	int		m_ChildCount;				//�θ� ��ȯ������ �ִ� �ڽ� ��
	std::vector<Transform*> m_Childs;	//�ڽĵ��� �迭

	Vector3	m_EulerAngles;				//���Ϸ� ������ ȸ���� �� ����

	Vector3	m_Forward;					//���� ���������� ����
	Vector3 m_Right;					//���� ���������� ������
	Vector3	m_Up;						//���� ���������� ����

	Vector3	m_LocalEulerAngles;			//�θ� ��ȯ�� ȸ���� �������� �ϴ� ���Ϸ� ������ ȸ��
	Vector3 m_LocalPosion;				//�θ� ��ȯ�� ������ �������� �� ��ȯ�� ��ġ
	Vector3 m_LocalRotation;			//�θ� ��ȯ�� ȸ���� �������� �� ��ȯ�� ȸ��

	Matrix m_LocalToWorldMatrix;		//���� �������� ���� �������� ��ȯ�ϴ� ��Ʈ���� read-only
	Matrix m_WorldToLocalMatrix;		//���� �������� ���� �������� ��ȯ�ϴ� ��Ʈ���� read-only

	Space m_Space;						//���� �� ����

	//��ӹ��� �������
public:
	//std::wstring name() Object
	//GameObject* m_gameObject;
	//Transform* m_transform;
	//std::wstring m_tag;
	//int		  m_Order;
public:
	Transform();
	virtual ~Transform();

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
	void	Start() override { m_IsStart = false; };
	void	FixedUpdate() override {};
	void	InputEvents() override {};
	void	Update(float dtime) override;
	void	LateUpdate() override {};
	void	PreRender() override {};
	void	Render() override {};
	void	PostRender() override {};

public:
	void	SetPosition(Vector3 position) { m_Position = position; };
	void	SetPosition(float position_x, float position_y, float position_z);
	Vector3 GetPosition() const { return m_Position; };

	void	SetRotation(Quaternion rotation) { m_Rotation = rotation; };
	void	SetRotation(float rotation_x, float rotation_y, float rotation_z, float rotation_w);
	Quaternion GetRotation() const { return m_Rotation; };

	void	SetScale(Vector3 scale) { m_Scale = scale; };
	void	SetScale(float scale_x, float scale_y, float scale_z);

	Vector3 GetScale() const { return m_Scale; };

public:
	//��� �ڳ��� �θ� �����Ѵ�.
	void		DetachChildren();
	//�ڽ��� ��ȯ�ϰų� ������ null �̴�.
	Transform* Find(std::wstring name);
	//�ڽĺ�ȯ�� �ε����� �Է¹޾� ��ȯ�Ѵ�.
	Transform* GetChild(int index);

	//������ ���� �������� ���� �������� ��ȯ�Ѵ�.
	Vector3 InverseTransformDirection(Vector3 direction);
	//��ġ�� ���� �������� ���� �������� ��ȯ�Ѵ�.
	Vector3 InverseTransformPoint(Vector3 position);
	//���� ���� �������� ���� �������� ��ȯ�Ѵ�.
	Vector3 InverseTransformVector(Vector3 vector);

	//�θ��� �ڽ����� ����
	bool IsChildOf(Transform* parent);
	//forward ���Ͱ� target �� ���� ��ġ�� ����Ű���� ��ȯ�� ȸ����Ų��.
	void LookAt(Transform traget);
	//���� ������Ʈ�� ȸ����Ų��.
	void Rotate(Vector3 eulers, Space relativeTo = Space::Self);
	//���� ������Ʈ�� ������� ȸ����Ų��.
	void RotateAround(Vector3 point, Vector3 axis, float angle);

	//�θ� ��ȯ�� ������Ų��.
	void SetParent(Transform* parent);
	void SetParent(Transform parent, bool worldPositionStays);
	//�θ� ��ȯ�� �����Ѵ�.
	void DetachParent();

	//��ġ�� ȸ���� �����Ѵ�.
	void SetPositionAndRotation(Vector3 position, Quaternion rotation);

	//������ ���� �������� ���� �������� ��ȯ�Ѵ�.
	Vector3 TransformDirection(Vector3 direction);
	//��ġ�� ���� �������� ���� �������� ��ȯ�Ѵ�.
	Vector3 TransformPoint(Vector3 point);
	//���͸� ���� �������� ���� �������� ��ȯ�Ѵ�.
	Vector3 TransformVector(Vector3 vector);

	//����� �Ÿ��� �̵��Ѵ�.
	void	Translate(Vector3 translation);
	void	Translate(Vector3 translation, Space relativeTo = Space::Self);

};

#endif