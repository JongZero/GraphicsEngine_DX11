#pragma once
#include <string>
#include <vector>

/// <summary>
/// �� �ϴ� �ι� ������ ��縦 ���� �ִ� Ŭ����
/// 2021.02.22
/// ������
/// </summary>
class JText
{
public:
	std::wstring Speaker;
	std::wstring Text;
};

/// <summary>
/// ��ũ��Ʈ�� ����ϱ� ���� Ŭ����
/// 2021.02.15
/// ������
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

	int DrawCount;		// �� �� ��ο� �� ������

	bool IsSpace;		// �������� �ƴ���

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

