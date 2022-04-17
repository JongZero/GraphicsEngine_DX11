#pragma once
#include <string>
#include <vector>

struct ID2D1Bitmap;

class Sprite
{
public:
	Sprite(float posX, float posY);
	~Sprite();

public:
	float m_PosX;
	float m_PosY;

	std::vector<ID2D1Bitmap*> m_BitmapVec;		// 여러 장의 텍스쳐
	ID2D1Bitmap* m_NowBitmap;
	int m_NowIndex;

public:
	void AddBitmap(ID2D1Bitmap* bmp);
	void SetBitmap(int i);
	void PrevBitmap();
	void NextBitmap();
};
