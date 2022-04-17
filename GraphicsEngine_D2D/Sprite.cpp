#include "pch.h"
#include "Sprite.h"

Sprite::Sprite(float posX, float posY)
	: m_NowBitmap(nullptr), m_NowIndex(0)
{

}

Sprite::~Sprite()
{

}

void Sprite::AddBitmap(ID2D1Bitmap* bmp)
{
	if (bmp)
	{
		m_BitmapVec.push_back(bmp);
		m_NowBitmap = m_BitmapVec[m_NowIndex];
	}
}

void Sprite::SetBitmap(int i)
{
	if (i < m_BitmapVec.size())
	{
		m_NowIndex = i;
		m_NowBitmap = m_BitmapVec[m_NowIndex];
	}
}

void Sprite::PrevBitmap()
{
	m_NowIndex--;

	if (m_NowIndex < 0)
	{
		m_NowIndex = m_BitmapVec.size() - 1;
	}

	m_NowBitmap = m_BitmapVec[m_NowIndex];
}

void Sprite::NextBitmap()
{
	m_NowIndex++;

	if (m_NowIndex >= m_BitmapVec.size())
	{
		m_NowIndex = 0;
	}

	m_NowBitmap = m_BitmapVec[m_NowIndex];
}
