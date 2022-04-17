#ifndef OBJECT_H
#define OBJECT_H
#pragma once

#include <string>
#include "IEvent.h"
#include "EMath.h"

/// <summary>
/// 2021.10.18 ������
/// ���ӿ�����Ʈ,������Ʈ���� �������̽� ������ �����Ѵ�.
/// �׷��Ƿ� ������Ʈ ���·δ� �������� �ʴ´�.
/// </summary>
class Object abstract :public IEvent
{
public:
	std::wstring m_Name;			//������Ʈ �̸�

public:
	Object();
	virtual ~Object();

public:
	virtual std::wstring GetName() const { return m_Name; };


public:
	//�ڽ��� �ı��Ѵ�.
	virtual void Destroy() = 0;
	//�� ��ȯ�� �ı����� �ʴ´�.
	virtual void DontDestroyOnLoad() = 0;
//	virtual void FindObjectOfType() = 0;
//	virtual void FindObjectsOfType() = 0;
	//�ν��Ͻ�ȭ �Ѵ�.
	virtual void Instantiate() = 0;
};


_inline static bool operator==(const Object& lhs, const Object& rhs)
{
	if (lhs.GetName() == rhs.GetName())
	{
		return true;
	}

	return false;
}

_inline static bool operator!=(const Object& lhs, const Object& rhs)
{
	if (lhs.GetName() == rhs.GetName())
	{
		return false;
	}

	return true;
}

#endif