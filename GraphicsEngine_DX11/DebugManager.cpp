#include "DebugManager.h"
#include "RenderTarget.h"
#include "GraphicsEngine_D2D.h"
#include "ClientSize.h"
#include "RenderTargetDrawer.h"
#include "DX11Core.h"
#include <tchar.h>

DebugManager::DebugManager()
{
	// ���� Ÿ�ٵ��� �׷��� ��ġ, ������ �̸� ���س���
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
	
	// ���� ���� Ÿ���� �׷��� ��ġ, ������
	m_FinalRenderTargetPosition = std::pair<float, float>(0, 0);
	m_FinalRenderTargetSize = std::pair<float, float>(0.5f, 0.5f);

	// �ؽ�Ʈ ���� ������ ��ġ (��, ��, ��, ��)
	m_TextPositionVec.resize(4);
}

DebugManager::~DebugManager()
{
	Clear();
}

DebugManager* DebugManager::GetInstance()
{
	// ����ƽ ���� �̱���
	// ���� static ��ü�� ���� ��� �������� ���� ��ü�ʹ� �޸� �ش� �Լ��� ó�� ȣ���ϴ� ������ �ʱ�ȭ�� ���ÿ� ������ �����
	// (�� ��ü�� �� ���� ������� ���� ��쿡�� ������ ���� ����)
	// �׷��鼭�� static ��ü�̱� ������ ���α׷� ���� �ñ��� ��ü�� �����ְ� ��
	// ���α׷� ���� �ÿ��� ���������� �ڵ����� �Ҹ��ڰ� ȣ���. �׷��Ƿ� �Ҹ��ڿ��� �ڿ� ������ �ϵ��� �ϰԵǸ� �ڿ� ������ �Ű澵 �ʿ䰡 ����
	// ������ : local static ������� ���� �̱��� ��ü�� �ٸ� ���� ��ü�� �Ҹ��ڿ��� ����� �� ������ �߻� (���� �幰����)
	// c++ ǥ�ؿ����� ���� ��ü�� ���� ������ ������� �ʾ���, �� �Ҹ� ������ ���ؼ��� ������� ����
	// -> ��, � ���� ��ü�� �Ҹ��ڿ��� �� �̱��� ��ü�� ����Ϸ� �� ��, �̱��� ��ü�� ���� �Ҹ��ߴٸ� ������ �߻��ϰԵ�
	// �̸� �ذ��ϱ� ���� �������, �Ǵн� �̱����̶�°� ����

	static DebugManager instance;	// C++11 ���ĺ��� Thread-safe�� �����
	return &instance;
} 

void DebugManager::Clear()
{
	// ť�� ��� ����ش�.
	while (!m_TextQueue.empty()) m_TextQueue.pop();
	while (!m_RenderTargetQueue.empty()) m_RenderTargetQueue.pop();
	while (!m_SRVQueue.empty()) m_SRVQueue.pop();
}

void DebugManager::DrawRenderTargets(DX11Core* dxCore, RenderTarget* final)
{
	size_t size = m_RenderTargetQueue.size();

	// �ִ� 12�������� (�� �̻��� �ʿ��ұ�?)
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

		// �ؽ�Ʈ ���
		float posX = m_RenderTargetPositionVec[i].first;
		float posY = -m_RenderTargetPositionVec[i].second;		// y ��ǥ�� ������ �ݴ����� ���

		// projection ��ȯ�� ���� ��ȯ�� ��ǥ�� -1.0f ~ 1.0f ������ ��
		// -1.0f ~ 1.0f ������ 0.0f ~ 1.0f (����) ������ ����
		posX = (posX + 1.0f - m_RenderTargetSize.first) / 2.0f;
		posY = (posY + 1.0f - m_RenderTargetSize.second) / 2.0f;

		// viewport ũ��� ������ ����Ͽ� client ��ǥ�� ��´�.
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
