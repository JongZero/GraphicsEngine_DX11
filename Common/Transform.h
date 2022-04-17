#ifndef TRANSFORM_H
#define TRANSFORM_H
#pragma once
#include "Component.h"

using namespace EMath;

//로컬 공간과 월드 공간을 구분한다.
enum class Space
{
	World = 0, Self
};

/// <summary>
/// 오브젝트들의 하이어라키 구성에 사용된다.
/// math 는 Utility.lib를 사용한다.
/// 2021.10.19 강주현
/// </summary>
class Transform : public Component
{

public:
	Vector3 m_Position;					//로컬에서의 위치
	Quaternion m_Rotation;				//로컬 공간에서의 변환의 회전
	Vector3 m_Scale;					//로컬에서의 크기

	Matrix m_WorldMatrix;				//월드 행렬

	Transform* m_Root;					//변환의 최상위 부모	
	Transform* m_Parent;				//변환의 부모
	int		m_ChildCount;				//부모 변환에서에 있는 자식 수
	std::vector<Transform*> m_Childs;	//자식들의 배열

	Vector3	m_EulerAngles;				//오일러 각도의 회전은 도 단위

	Vector3	m_Forward;					//월드 공간에서의 앞쪽
	Vector3 m_Right;					//월드 공간에서의 오른쪽
	Vector3	m_Up;						//월드 공간에서의 위쪽

	Vector3	m_LocalEulerAngles;			//부모 변환의 회전을 기준으로 하는 오일러 각도의 회전
	Vector3 m_LocalPosion;				//부모 변환을 기준을 기준으로 한 변환의 위치
	Vector3 m_LocalRotation;			//부모 변환의 회전을 기준으로 한 변환의 회전

	Matrix m_LocalToWorldMatrix;		//로컬 공간에서 월드 공간으로 변환하는 매트릭스 read-only
	Matrix m_WorldToLocalMatrix;		//월드 공간에서 로컬 공간으로 변환하는 매트릭스 read-only

	Space m_Space;						//현재 내 공간

	//상속받은 멤버변수
public:
	//std::wstring name() Object
	//GameObject* m_gameObject;
	//Transform* m_transform;
	//std::wstring m_tag;
	//int		  m_Order;
public:
	Transform();
	virtual ~Transform();

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
	//모든 자녀의 부모를 해제한다.
	void		DetachChildren();
	//자식을 반환하거나 없으면 null 이다.
	Transform* Find(std::wstring name);
	//자식변환의 인덱스를 입력받아 반환한다.
	Transform* GetChild(int index);

	//방향을 월드 공간에서 로컬 공간으로 변환한다.
	Vector3 InverseTransformDirection(Vector3 direction);
	//위치를 월드 공간에서 로컬 공간으로 변환한다.
	Vector3 InverseTransformPoint(Vector3 position);
	//벡터 월드 공간에서 로컬 공간으로 변환한다.
	Vector3 InverseTransformVector(Vector3 vector);

	//부모의 자식인지 여부
	bool IsChildOf(Transform* parent);
	//forward 벡터가 target 의 현재 위치를 가리키도록 변환을 회전시킨다.
	void LookAt(Transform traget);
	//게임 오브젝트를 회전시킨다.
	void Rotate(Vector3 eulers, Space relativeTo = Space::Self);
	//게임 오브젝트를 축단위로 회전시킨다.
	void RotateAround(Vector3 point, Vector3 axis, float angle);

	//부모 변환을 지정시킨다.
	void SetParent(Transform* parent);
	void SetParent(Transform parent, bool worldPositionStays);
	//부모 변환을 해제한다.
	void DetachParent();

	//위치와 회전을 설정한다.
	void SetPositionAndRotation(Vector3 position, Quaternion rotation);

	//방향을 로컬 공간에서 월드 공간으로 변환한다.
	Vector3 TransformDirection(Vector3 direction);
	//위치를 로컬 공간에서 월드 공간으로 변환한다.
	Vector3 TransformPoint(Vector3 point);
	//벡터를 로컬 공간에서 월드 공간으로 변환한다.
	Vector3 TransformVector(Vector3 vector);

	//방향과 거리로 이동한다.
	void	Translate(Vector3 translation);
	void	Translate(Vector3 translation, Space relativeTo = Space::Self);

};

#endif