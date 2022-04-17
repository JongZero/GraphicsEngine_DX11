#ifndef OBJECT_H
#define OBJECT_H
#pragma once

#include <string>
#include "IEvent.h"
#include "EMath.h"

/// <summary>
/// 2021.10.18 강주현
/// 게임오브젝트,컴포너트들의 인터페이스 역할을 수행한다.
/// 그러므로 오브젝트 상태로는 존재하지 않는다.
/// </summary>
class Object abstract :public IEvent
{
public:
	std::wstring m_Name;			//오브젝트 이름

public:
	Object();
	virtual ~Object();

public:
	virtual std::wstring GetName() const { return m_Name; };


public:
	//자신을 파괴한다.
	virtual void Destroy() = 0;
	//씬 전환시 파괴되지 않는다.
	virtual void DontDestroyOnLoad() = 0;
//	virtual void FindObjectOfType() = 0;
//	virtual void FindObjectsOfType() = 0;
	//인스턴스화 한다.
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