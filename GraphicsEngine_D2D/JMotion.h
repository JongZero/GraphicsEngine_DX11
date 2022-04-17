#pragma once

#include <vector>
//#include <atlstr.h> // MFC, 2022년 1월 - CString, CPoint등의 MFC 미사용으로 삭제

#include "JSprite.h"

class ID2D1Bitmap;

// 스프라이트 시트 사용
class JMotion
{
public:
	//CString ObjectName;
	//CString Name;
	std::vector<JSprite> SpriteVec;

	int Frame;

	int DrawIndex;
	float DrawTime;

	int PlayCount;

public:
	JMotion();
	~JMotion() {};

public:
	void Reset();
	void Update(float dTime, float speed = 1.0f);

	void ChangeMotionFrame(int frame);
	void NextMotionFrame();
	void Play(float dTime, float speed = 1.0f);
};

// 스프라이트 시트를 사용하지 않는 이미지(애니메이션)
class JImage
{
public:
	//CString ObjectName;
	ID2D1Bitmap* Bitmap;

	int Frame;

	int DrawIndex;
	float DrawTime;

	bool PlayOnlyOnce;

public:
	JImage(int frame);
	~JImage() {};

public:
	void Reset();
	void Update(float dTime, float speed = 1.0f);
	
	void ChangeFrame(int frame);
	void NextFrame();
	bool Play(float dTime, float speed = 1.0f);
	bool PlayAndReset(float dTime, float speed /*= 1.0f*/);
};