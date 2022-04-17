#pragma once
#ifndef IEVENT_H
#define IEVENT_H

#include "MacroDefine.h"

/// <summary>
/// ������ �̺�Ʈ���� ����Ѵ�.
/// 2021.10.27 ������
/// </summary>
class IEvent abstract
{
public:
	IEvent() {};
	virtual ~IEvent() = default;

public:
	//��Ȱ���� ������Ʈ�� Ȱ��ȭ �� �� �� �� ȣ��ȴ�.
	virtual void	Awake() = 0;
	//Update�� ó�� ȣ��Ǳ� ������ ȣ��ȴ�.
	virtual void	Start() = 0;
	//��������� �̺�Ʈ���� ������Ʈ�Ѵ�.
	virtual void	FixedUpdate() = 0;
	//�Է� �̺�Ʈ�� ������Ʈ�Ѵ�.
	virtual void	InputEvents() = 0;
	//�� ������ �ʼ� ������ ����ȴ�
	virtual void	Update(float dtime) = 0;
	//Update�� ����� �� ����ȴ� -> ī�޶� ���� ����
	virtual void	LateUpdate() = 0;
	//Render ȣ�� ���� ȣ��ȴ�.
	virtual void	PreRender() = 0;
	//
	virtual void	Render() = 0;
	//Render ȣ�� �Ŀ� ȣ��ȴ�.
	virtual void	PostRender() = 0;
};

#endif
