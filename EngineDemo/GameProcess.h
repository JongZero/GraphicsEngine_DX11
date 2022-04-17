#pragma once
#include <memory>
class EggEngine;
class InputManager;

class GameProcess
{
private:
	HWND m_hWnd;

	std::unique_ptr<EggEngine> m_pEggEngine;
	InputManager* m_pInputManager;

public:
	GameProcess();
	~GameProcess();

public:
	bool Initialize(HINSTANCE hInstance);

	void MessageLoop();
	void GameLoop();

private:


private:
	int m_ClientWidth;
	int m_ClientHeight;

	bool m_IsAppPaused;
	bool m_IsMinimized;
	bool m_IsMaximized;
	bool m_IsResizing;
	
private:
	void RecalcWindowSize();

public:
	LRESULT CALLBACK MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};