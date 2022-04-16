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
/// 이번 프레임에 그려야 할 디버그용 객체들을 갖고 있다가
/// 3D 렌더링이 모두 끝난 후 디버그 모드라면 디버그 객체를 그리거나 2D렌더러에 넘긴다.
/// 렌더 타겟, 텍스트, UI등을 갖고 있는다.
/// 렌더러, 리소스 매니저, 애니메이터등등 디버그 정보들은 언제 어디서 필요할 지 모르므로
/// 싱글톤으로 만들어 전역성을 확보해 개발 편의성을 높인다.
/// 2022. 01. 10 정종영
/// </summary>
class DebugManager
{
private:
	DebugManager();
	~DebugManager();

public:
	static DebugManager* GetInstance();

private:
	std::queue<TextInfo> m_TextQueue;				// 이번 프레임에 그려질 디버그용 텍스트 큐
	std::queue<RenderTarget*> m_RenderTargetQueue;	// 이번 프레임에 그려질 디버그용 렌더타겟 큐,	최대 12개, 
													// 들어 오는 순서대로 정해진 위치에 그려짐
	std::queue<std::pair<ID3D11ShaderResourceView*, std::wstring>> m_SRVQueue;

	std::vector<std::pair<float, float>> m_RenderTargetPositionVec;		// 렌더 타겟들이 그려질 위치들
	std::pair<float, float> m_RenderTargetSize;							// 렌더 타겟들의 사이즈

	std::pair<float, float> m_FinalRenderTargetPosition;				// 최종 렌더 타겟이 그려질 위치
	std::pair<float, float> m_FinalRenderTargetSize;					// 최종 렌더 타겟의 사이즈

	std::vector<std::pair<float, float>> m_TextPositionVec;		// 텍스트들이 그려질 위치들

public:
	void AddText(TextInfo s) { m_TextQueue.push(s); }
	void AddRenderTarget(RenderTarget* rt) { m_RenderTargetQueue.push(rt); }
	void AddSRV(ID3D11ShaderResourceView* srv, std::wstring name) { m_SRVQueue.push(std::pair<ID3D11ShaderResourceView*, std::wstring>(srv, name)); }

public:
	void Clear();									// 렌더러에서 매 프레임마다 렌더링이 모두 끝난 뒤 호출, 큐를 비워줌
	void DrawRenderTargets(DX11Core* dxCore, RenderTarget* final);	// 렌더 타겟을 그림
													// 그려지는 순서, 가운데는 최종적으로 그려진 화면
													// ㅁㅁㅁㅁ	0	1	2	3
													// ㅁ■■ㅁ	11	최	종	4
													// ㅁ■■ㅁ	10	화	면	5
													// ㅁㅁㅁㅁ	9	8	7	6
	void DrawTexts(D2DRenderer* d2dRenderer);

private:
	void ResetTextPosition();
};
