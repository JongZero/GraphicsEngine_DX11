#pragma once
#include <windows.h>

struct DXInput;

/// <summary>
/// KeyBoard, Mouse Input Utility
/// 
/// InputManager에서 DXTK를 숨기기 위해 만들어짐
/// 2021. 11. 05 Hakgeum
/// </summary>
class InputUtility
{
public:
	InputUtility();
	~InputUtility();

public:
	void Initialize(HWND hWnd);
	void Update();

#pragma region Mouse
public:
	bool CheckMouseConnect();
	void SetMouseMode(bool defaultMode);
	void SetCursorVisible(bool visible);
	bool IsButtonUp(int btn);
	bool IsButtonHeld(int btn);
	bool IsButtonReleased(int btn);
	bool IsButtonPressed(int btn);
	void MouseReset();
	void MouseProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);
#pragma endregion

#pragma region Keyboard
public:
	bool CheckKeyboardConnect();
	void KeyboardReset();
	bool IsKeyDown(unsigned char key);
	bool IsKeyUp(unsigned char key);
	bool IsKeyPressed(unsigned char key);
	bool IsKeyReleased(unsigned char key);
	void KeyboardProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);
#pragma endregion

private:
	DXInput* m_pDXInput;
};
