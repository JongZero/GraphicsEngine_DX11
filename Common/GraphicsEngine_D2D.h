#pragma once

#ifdef _WIN64
	#ifdef _DEBUG
		#pragma comment(lib, "GraphicsEngine_D2D_x64_debug")
	#else
		#pragma comment(lib, "GraphicsEngine_D2D_x64_release")	
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "GraphicsEngine_D2D_x86_debug")
	#else
		#pragma comment(lib, "GraphicsEngine_D2D_x86_release")
	#endif
#endif

#include <d2d1.h>
#include <d2d1helper.h>

// 2차원 벡터사용
#include <dcommon.h>
#include <dwrite.h>			// writefactory
#include <stdio.h>

#include <wincodec.h>
#include <xstring>

#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib") // WIC
using namespace D2D1;

#include <string>

class Sprite;

/// Direct2D를 이용한 엔진
/// 2021.01.19
/// 정종영
class D2DRenderer
{
public:
	D2DRenderer(HWND hwnd, IDXGISwapChain* dxSwapChain);
	~D2DRenderer();

private:
	HWND m_Hwnd;
	ID2D1Factory* m_pFactory;
	IDXGISwapChain* m_DXSwapChain;			// dx의 스왑체인
	ID2D1RenderTarget* m_pRenderTarget;

	// 텍스트 팩토리
	IDWriteFactory* m_pDWriteFactory;
	IDWriteTextFormat* m_pTextFormat;

	// 폰트 관련
	std::wstring m_MainFont;
	FLOAT m_MainFontSize;

	// 이미지 팩토리
	IWICImagingFactory* m_pImageFactory;

	// 현재 브러쉬
	ID2D1SolidColorBrush* m_pNowBrush;

public:
	void Initialize(HWND hwnd, IDXGISwapChain* dxSwapChain);
	void Release();
	void ReleaseRenderTarget();		// DX의 스왑체인의 리사이즈를 위해 묶여있는 렌더 타겟을 릴리즈해준다.
	bool OnResize();

	void BeginRender();
	void EndRender();

	void SetBrushColor(float r, float g, float b, float a);
	void SetBrushColor(D2D1::ColorF color);
	void SetBrushColor(D2D1_COLOR_F color);

	void SetFontSize(float fontSize);
	void SetFont(std::wstring fontName);
	void SetFont(std::wstring fontName, float fontSize);

public:
	/// 기본 도형 출력
	void DrawLine(float x1, float y1, float x2, float y2);
	void DrawLine(float x1, float y1, float x2, float y2, ColorF color);

	void DrawRectangle(float x1, float y1, float x2, float y2);
	void DrawRectangle(float x1, float y1, float x2, float y2, ColorF color);

	void DrawFillRectangle(float x1, float y1, float x2, float y2);
	void DrawFillRectangle(float x1, float y1, float x2, float y2, ColorF color);

	void DrawEllipse(float x, float y, float size);
	void DrawEllipse(float x1, float y1, float x2, float y2);
	void DrawEllipse(float x, float y, float size, ColorF color);
	void DrawEllipse(float x1, float y1, float x2, float y2, ColorF color);

	void DrawFillEllipse(float x1, float y1, float x2, float y2);
	void DrawFillEllipse(float x, float y, float size, ColorF color);
	void DrawFillEllipse(float x1, float y1, float x2, float y2, ColorF color);

	/// 문자 출력
	void DrawText(float x, float y, const WCHAR* pch, ...);
	void DrawText(float x, float y, ColorF color, const WCHAR* pch, ...);
	
	void DrawText(float x, float y, float fontSize, const WCHAR* pch, ...);
	void DrawText(float x, float y, float fontSize, ColorF color, const WCHAR* pch, ...);

	void DrawText(float x, float y, const WCHAR* font, const WCHAR* pch, ...);
	void DrawText(float x, float y, const WCHAR* font, ColorF color, const WCHAR* pch, ...);

	void DrawText(float x, float y, const WCHAR* font, float fontSize, const WCHAR* pch, ...);
	void DrawText(float x, float y, const WCHAR* font, float fontSize, ColorF color, const WCHAR* pch, ...);

	// 스크립트를 출력
	void DrawText(float x, float y, int sizeX, int sizeY, const WCHAR* pch, ...);
	//void DrawText(float x, float y, int sizeX, int sizeY, JScript script);

	void DrawText(float x, float y, int sizeX, int sizeY, int fontSize, ColorF color, const WCHAR* pch, ...);
	void DrawText(float x, float y, int sizeX, int sizeY, const WCHAR* font, int fontSize, ColorF color, const WCHAR* pch, ...);

	// 외곽선
	void DrawOutlineText(float x, float y, ColorF color, const WCHAR* pch, ...);

	/// 스프라이트 로드, 출력
	HRESULT LoadBitmapFromFile(PCWSTR uri, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap);	
	void DrawSprite(Sprite* sprite, float x, float y, float opacity = 1.0f);
	//void DrawSprite(JImage* image, float x, float y, float opacity = 1.0f);
	//void DrawSprite(ID2D1Bitmap* sprite, float x, float y, float opacity = 1.0f);
	//void DrawSprite(ID2D1Bitmap* spriteSheet, JMotion* motion, float x, float y, float opacity = 1.0f);
};