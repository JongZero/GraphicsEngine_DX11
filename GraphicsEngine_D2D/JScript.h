#pragma once
#include <string>
#include <vector>

/// <summary>
/// 말 하는 인물 정보와 대사를 갖고 있는 클래스
/// 2021.02.22
/// 정종영
/// </summary>
class JText
{
public:
	std::wstring Speaker;
	std::wstring Text;
};

/// <summary>
/// 스크립트를 출력하기 위한 클래스
/// 2021.02.15
/// 정종영
/// </summary>
class JScript
{
public:
	std::vector<JText> TextVec;

	std::wstring NowSpeacker;
	std::wstring NowText;

	int NowTextIndex;
	bool IsNowTextEnd;

	bool IsAllTextEnd;

	int DrawIndex;
	float DrawTime;

	int DrawCount;		// 몇 번 드로우 할 것인지

	bool IsSpace;		// 공백인지 아닌지

public:
	JScript();
	~JScript() {};

public:
	void NextIndex();
	void DrawAllNowText();
	void ChangeText(int index);
	std::wstring GetText(int index);
	std::wstring GetSpeaker(int index);
	void Reset();
	void Update(float dTime, float speed = 1.0f);
};

