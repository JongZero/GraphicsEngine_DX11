#include "DebugManager.h"
#include "RenderTarget.h"
#include "GraphicsEngine_D2D.h"
#include "ClientSize.h"
#include "RenderTargetDrawer.h"
#include "DX11Core.h"
#include <tchar.h>

DebugManager::DebugManager()
{
	// 렌더 타겟들이 그려질 위치, 사이즈 미리 정해놓음
	m_RenderTargetPositionVec.resize(12);
	m_RenderTargetPositionVec[0] = std::pair<float, float>(-0.75f, 0.75f);
	m_RenderTargetPositionVec[1] = std::pair<float, float>(-0.25f, 0.75f);
	m_RenderTargetPositionVec[2] = std::pair<float, float>(0.25f, 0.75f);
	m_RenderTargetPositionVec[3] = std::pair<float, float>(0.75f, 0.75f);
	m_RenderTargetPositionVec[4] = std::pair<float, float>(0.75f, 0.25f);
	m_RenderTargetPositionVec[5] = std::pair<float, float>(0.75f, -0.25f);
	m_RenderTargetPositionVec[6] = std::pair<float, float>(0.75f, -0.75f);
	m_RenderTargetPositionVec[7] = std::pair<float, float>(0.25f, -0.75f);
	m_RenderTargetPositionVec[8] = std::pair<float, float>(-0.25f, -0.75f);
	m_RenderTargetPositionVec[9] = std::pair<float, float>(-0.75f, -0.75f);
	m_RenderTargetPositionVec[10] = std::pair<float, float>(-0.75f, -0.25f);
	m_RenderTargetPositionVec[11] = std::pair<float, float>(-0.75f, 0.25f);

	m_RenderTargetSize = std::pair<float, float>(0.25f, 0.25f);
	
	// 최종 렌더 타겟이 그려질 위치, 사이즈
	m_FinalRenderTargetPosition = std::pair<float, float>(0, 0);
	m_FinalRenderTargetSize = std::pair<float, float>(0.5f, 0.5f);

	// 텍스트 들의 정해진 위치 (상, 하, 좌, 우)
	m_TextPositionVec.resize(4);
}

DebugManager::~DebugManager()
{
	Clear();
}

DebugManager* DebugManager::GetInstance()
{
	// 스테틱 지역 싱글톤
	// 지역 static 객체로 만들 경우 전역으로 만든 객체와는 달리 해당 함수를 처음 호출하는 시점에 초기화와 동시에 생성이 진행됨
	// (이 객체를 한 번도 사용하지 않을 경우에는 생성이 되지 않음)
	// 그러면서도 static 객체이기 때문에 프로그램 종료 시까지 객체가 남아있게 됨
	// 프로그램 종료 시에는 마찬가지로 자동으로 소멸자가 호출됨. 그러므로 소멸자에서 자원 해제를 하도록 하게되면 자원 관리도 신경쓸 필요가 없음
	// 문제점 : local static 방식으로 만든 싱글톤 객체를 다른 전역 객체의 소멸자에서 사용할 시 문제가 발생 (극히 드물지만)
	// c++ 표준에서는 전역 객체의 생성 순서를 명시하지 않았음, 또 소멸 순서에 대해서도 명시하지 않음
	// -> 즉, 어떤 전역 객체가 소멸자에서 이 싱글톤 객체를 사용하려 할 때, 싱글톤 객체가 먼저 소멸했다면 문제가 발생하게됨
	// 이를 해결하기 위한 방안으로, 피닉스 싱글톤이라는게 있음

	static DebugManager instance;	// C++11 이후부턴 Thread-safe가 보장됨
	return &instance;
} 

void DebugManager::Clear()
{
	// 큐를 모두 비워준다.
	while (!m_TextQueue.empty()) m_TextQueue.pop();
	while (!m_RenderTargetQueue.empty()) m_RenderTargetQueue.pop();
	while (!m_SRVQueue.empty()) m_SRVQueue.pop();
}

void DebugManager::DrawRenderTargets(DX11Core* dxCore, RenderTarget* final)
{
	size_t size = m_RenderTargetQueue.size();

	// 최대 12개까지만 (그 이상이 필요할까?)
	if (size > 12)
		size = 12;

	int i = 0;
	while (!m_RenderTargetQueue.empty())
	{
		RenderTarget* rt = m_RenderTargetQueue.front();
		rt->SetWorld(m_RenderTargetSize.first, m_RenderTargetSize.second,
			m_RenderTargetPositionVec[i].first, m_RenderTargetPositionVec[i].second);
		rt->Draw(dxCore->GetDC());
		m_RenderTargetQueue.pop();
		i++;
	}

	while (!m_SRVQueue.empty())
	{
		std::pair<ID3D11ShaderResourceView*, std::wstring> srv = m_SRVQueue.front();
		EMath::Matrix world = EMath::Matrix::CreateScale(m_RenderTargetSize.first, m_RenderTargetSize.second, 0)
								* EMath::Matrix::CreateTranslation(m_RenderTargetPositionVec[i].first, m_RenderTargetPositionVec[i].second, 0);

		RenderTargetDrawer::DrawRenderTarget(dxCore->GetDC(), srv.first, world);

		// 텍스트 출력
		float posX = m_RenderTargetPositionVec[i].first;
		float posY = -m_RenderTargetPositionVec[i].second;		// y 좌표의 방향이 반대임을 고려

		// projection 변환에 의해 변환된 좌표는 -1.0f ~ 1.0f 사이의 값
		// -1.0f ~ 1.0f 범위를 0.0f ~ 1.0f (비율) 범위로 변경
		posX = (posX + 1.0f - m_RenderTargetSize.first) / 2.0f;
		posY = (posY + 1.0f - m_RenderTargetSize.second) / 2.0f;

		// viewport 크기와 비율을 계산하여 client 좌표를 얻는다.
		posX = ClientSize::GetWidth() * (posX);
		posY = ClientSize::GetHeight() * (posY);

		AddText(TextInfo(
			EMath::Vector2(posX, posY),
			D2D1::ColorF::Black, (TCHAR*)srv.second.c_str()));
		
		i++;
		m_SRVQueue.pop();
	}

	final->SetWorld(m_FinalRenderTargetSize.first, m_FinalRenderTargetSize.second,
		m_FinalRenderTargetPosition.first, m_FinalRenderTargetPosition.second);
	final->Draw(dxCore->GetDC());
}

void DebugManager::DrawTexts(D2DRenderer* d2dRenderer)
{
	ResetTextPosition();
	std::pair<float, float> pos = { 0, 0 };

	size_t size = m_TextQueue.size();
	for (int i = 0; i < size; i++)
	{
		const TextInfo& text = m_TextQueue.front();
		
		if (text.PositionInfo == eTextPositionInfo::Free)
		{
			pos.first = text.Position.x;
			pos.second = text.Position.y;
		}
		else
		{
			pos.first = m_TextPositionVec[static_cast<int>(text.PositionInfo)].first;
			pos.second = m_TextPositionVec[static_cast<int>(text.PositionInfo)].second;

			m_TextPositionVec[static_cast<int>(text.PositionInfo)].second += 20;
		}

		d2dRenderer->DrawText(pos.first, pos.second, text.Color, text.Text);
		m_TextQueue.pop();
	}
}

void DebugManager::ResetTextPosition()
{
	int width = ClientSize::GetWidth();
	int height = ClientSize::GetHeight();

	m_TextPositionVec[static_cast<int>(eTextPositionInfo::Up)] = std::pair<float, float>(width / 2 - 100, 0);
	m_TextPositionVec[static_cast<int>(eTextPositionInfo::Down)] = std::pair<float, float>(width / 2 - 100, height - 400);
	m_TextPositionVec[static_cast<int>(eTextPositionInfo::Left)] = std::pair<float, float>(0, height / 2 - 400);
	m_TextPositionVec[static_cast<int>(eTextPositionInfo::Right)] = std::pair<float, float>(width - 400, height / 2 - 400);
}

TextInfo::TextInfo(eTextPositionInfo posInfo, TCHAR* text, ...)
	: PositionInfo(posInfo)
{
	va_list vl;
	va_start(vl, text);
	_vstprintf(Text, 1024, text, vl);
	va_end(vl);
}

TextInfo::TextInfo(EMath::Vector2 position, TCHAR* text, ...)
	: Position(position)
{
	va_list vl;
	va_start(vl, text);
	_vstprintf(Text, 1024, text, vl);
	va_end(vl);
}

TextInfo::TextInfo(EMath::Vector2 position, D2D1::ColorF color, TCHAR* text, ...)
	: Position(position), Color(color)
{
	va_list vl;
	va_start(vl, text);
	_vstprintf(Text, 1024, text, vl);
	va_end(vl);
}

TextInfo::TextInfo(EMath::Vector2 position, float size, D2D1::ColorF color, TCHAR* text, ...)
	: Position(position), Size(size), Color(color)
{
	va_list vl;
	va_start(vl, text);
	_vstprintf(Text, 1024, text, vl);
	va_end(vl);
}

TextInfo::TextInfo(eTextPositionInfo posInfo, float size, TCHAR* text, ...)
	: PositionInfo(posInfo), Size(size)
{
	va_list vl;
	va_start(vl, text);
	_vstprintf(Text, 1024, text, vl);
	va_end(vl);
}

TextInfo::TextInfo(eTextPositionInfo posInfo, D2D1::ColorF color, TCHAR* text, ...)
	: PositionInfo(posInfo), Color(color)
{
	va_list vl;
	va_start(vl, text);
	_vstprintf(Text, 1024, text, vl);
	va_end(vl);
}

TextInfo::TextInfo(eTextPositionInfo posInfo, float size, D2D1::ColorF color, TCHAR* text, ...)
	: PositionInfo(posInfo), Size(size), Color(color)
{
	va_list vl;
	va_start(vl, text);
	_vstprintf(Text, 1024, text, vl);
	va_end(vl);
}
