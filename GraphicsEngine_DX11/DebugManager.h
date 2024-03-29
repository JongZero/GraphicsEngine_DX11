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
/// 戚腰 覗傾績拭 益形醤 拝 巨獄益遂 梓端級聖 握壱 赤陥亜
/// 3D 兄希元戚 乞砧 魁貝 板 巨獄益 乞球虞檎 巨獄益 梓端研 益軒暗蟹 2D兄希君拭 角延陥.
/// 兄希 展為, 努什闘, UI去聖 握壱 赤澗陥.
/// 兄希君, 軒社什 古艦煽, 蕉艦五戚斗去去 巨獄益 舛左級精 情薦 嬢巨辞 琶推拝 走 乞牽糠稽
/// 縮越宕生稽 幻級嬢 穿蝕失聖 溌左背 鯵降 畷税失聖 株昔陥.
/// 2022. 01. 10 舛曽慎
/// </summary>
class DebugManager
{
private:
	DebugManager();
	~DebugManager();

public:
	static DebugManager* GetInstance();

private:
	std::queue<TextInfo> m_TextQueue;				// 戚腰 覗傾績拭 益形霜 巨獄益遂 努什闘 泥
	std::queue<RenderTarget*> m_RenderTargetQueue;	// 戚腰 覗傾績拭 益形霜 巨獄益遂 兄希展為 泥,	置企 12鯵, 
													// 級嬢 神澗 授辞企稽 舛背遭 是帖拭 益形像
	std::queue<std::pair<ID3D11ShaderResourceView*, std::wstring>> m_SRVQueue;

	std::vector<std::pair<float, float>> m_RenderTargetPositionVec;		// 兄希 展為級戚 益形霜 是帖級
	std::pair<float, float> m_RenderTargetSize;							// 兄希 展為級税 紫戚綜

	std::pair<float, float> m_FinalRenderTargetPosition;				// 置曽 兄希 展為戚 益形霜 是帖
	std::pair<float, float> m_FinalRenderTargetSize;					// 置曽 兄希 展為税 紫戚綜

	std::vector<std::pair<float, float>> m_TextPositionVec;		// 努什闘級戚 益形霜 是帖級

public:
	void AddText(TextInfo s) { m_TextQueue.push(s); }
	void AddRenderTarget(RenderTarget* rt) { m_RenderTargetQueue.push(rt); }
	void AddSRV(ID3D11ShaderResourceView* srv, std::wstring name) { m_SRVQueue.push(std::pair<ID3D11ShaderResourceView*, std::wstring>(srv, name)); }

public:
	void Clear();									// 兄希君拭辞 古 覗傾績原陥 兄希元戚 乞砧 魁貝 及 硲窒, 泥研 搾趨捜
	void DrawRenderTargets(DX11Core* dxCore, RenderTarget* final);	// 兄希 展為聖 益顕
													// 益形走澗 授辞, 亜錘汽澗 置曽旋生稽 益形遭 鉢檎
													// けけけけ	0	1	2	3
													// け＝＝け	11	置	曽	4
													// け＝＝け	10	鉢	檎	5
													// けけけけ	9	8	7	6
	void DrawTexts(D2DRenderer* d2dRenderer);

private:
	void ResetTextPosition();
};
