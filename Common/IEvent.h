#pragma once
#ifndef IEVENT_H
#define IEVENT_H

#include "MacroDefine.h"

/// <summary>
/// 게임의 이벤트들을 담당한다.
/// 2021.10.27 강주현
/// </summary>
class IEvent abstract
{
public:
	IEvent() {};
	virtual ~IEvent() = default;

public:
	//비활성된 오브젝트가 활성화 될 시 한 번 호출된다.
	virtual void	Awake() = 0;
	//Update가 처음 호출되기 직전에 호출된다.
	virtual void	Start() = 0;
	//물리기반의 이벤트들을 업데이트한다.
	virtual void	FixedUpdate() = 0;
	//입력 이벤트를 업데이트한다.
	virtual void	InputEvents() = 0;
	//한 프레임 필수 적으로 실행된다
	virtual void	Update(float dtime) = 0;
	//Update가 실행된 후 실행된다 -> 카메라에 적용 예정
	virtual void	LateUpdate() = 0;
	//Render 호출 전에 호출된다.
	virtual void	PreRender() = 0;
	//
	virtual void	Render() = 0;
	//Render 호출 후에 호출된다.
	virtual void	PostRender() = 0;
};

#endif
