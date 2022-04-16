#pragma once
#include <queue>
#include <vector>
#include <string>
#include <utility>		// for pair
#include <d2d1helper.h>
#include "EMath.h"

class DX11Core;
class RenderTarget;
class D2DRenderer;
struct ID3D11ShaderResourceView;

enum class eTextPositionInfo
{
	Up,
	Down,
	Left,
	Right,
	Free,
};

struct TextInfo
{
public:
	TextInfo(eTextPositionInfo posInfo, TCHAR* text, ...);
	TextInfo(EMath::Vector2 position, TCHAR* text, ...);
	TextInfo(EMath::Vector2 position, D2D1::ColorF color, TCHAR* text, ...);
	TextInfo(EMath::Vector2 position, float size, D2D1::ColorF color, TCHAR* text, ...);
	TextInfo(eTextPositionInfo posInfo, float size, TCHAR* text, ...);
	TextInfo(eTextPositionInfo posInfo, D2D1::ColorF color, TCHAR* text, ...);
	TextInfo(eTextPositionInfo posInfo, float size, D2D1::ColorF color, TCHAR* text, ...);

public:
	eTextPositionInfo PositionInfo = eTextPositionInfo::Free;
	EMath::Vector2 Position = { 0, 0 };
	float Size = 10.0f;
	D2D1::ColorF Color = D2D1::ColorF::White;
	TCHAR Text[1024];
};

/// <summary>
/// �̹� �����ӿ� �׷��� �� ����׿� ��ü���� ���� �ִٰ�
/// 3D �������� ��� ���� �� ����� ����� ����� ��ü�� �׸��ų� 2D�������� �ѱ��.
/// ���� Ÿ��, �ؽ�Ʈ, UI���� ���� �ִ´�.
/// ������, ���ҽ� �Ŵ���, �ִϸ����͵�� ����� �������� ���� ��� �ʿ��� �� �𸣹Ƿ�
/// �̱������� ����� �������� Ȯ���� ���� ���Ǽ��� ���δ�.
/// 2022. 01. 10 ������
/// </summary>
class DebugManager
{
private:
	DebugManager();
	~DebugManager();

public:
	static DebugManager* GetInstance();

private:
	std::queue<TextInfo> m_TextQueue;				// �̹� �����ӿ� �׷��� ����׿� �ؽ�Ʈ ť
	std::queue<RenderTarget*> m_RenderTargetQueue;	// �̹� �����ӿ� �׷��� ����׿� ����Ÿ�� ť,	�ִ� 12��, 
													// ��� ���� ������� ������ ��ġ�� �׷���
	std::queue<std::pair<ID3D11ShaderResourceView*, std::wstring>> m_SRVQueue;

	std::vector<std::pair<float, float>> m_RenderTargetPositionVec;		// ���� Ÿ�ٵ��� �׷��� ��ġ��
	std::pair<float, float> m_RenderTargetSize;							// ���� Ÿ�ٵ��� ������

	std::pair<float, float> m_FinalRenderTargetPosition;				// ���� ���� Ÿ���� �׷��� ��ġ
	std::pair<float, float> m_FinalRenderTargetSize;					// ���� ���� Ÿ���� ������

	std::vector<std::pair<float, float>> m_TextPositionVec;		// �ؽ�Ʈ���� �׷��� ��ġ��

public:
	void AddText(TextInfo s) { m_TextQueue.push(s); }
	void AddRenderTarget(RenderTarget* rt) { m_RenderTargetQueue.push(rt); }
	void AddSRV(ID3D11ShaderResourceView* srv, std::wstring name) { m_SRVQueue.push(std::pair<ID3D11ShaderResourceView*, std::wstring>(srv, name)); }

public:
	void Clear();									// ���������� �� �����Ӹ��� �������� ��� ���� �� ȣ��, ť�� �����
	void DrawRenderTargets(DX11Core* dxCore, RenderTarget* final);	// ���� Ÿ���� �׸�
													// �׷����� ����, ����� ���������� �׷��� ȭ��
													// ��������	0	1	2	3
													// ����ᤱ	11	��	��	4
													// ����ᤱ	10	ȭ	��	5
													// ��������	9	8	7	6
	void DrawTexts(D2DRenderer* d2dRenderer);

private:
	void ResetTextPosition();
};
