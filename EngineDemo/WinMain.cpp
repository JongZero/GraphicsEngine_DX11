#include "framework.h"
#include "Resource.h"
#include "GameProcess.h"

// 윈도 프로시저
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// 윈 메인
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					  _In_opt_ HINSTANCE hPrevInstance,
					  _In_ LPWSTR lpCmdLine,
					  _In_ int nCmdShow)
{
	// 애플리케이션 초기화를 수행합니다:
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAMEENGINEDEMO));

	GameProcess* _GameProcess = new GameProcess();

	if (!_GameProcess->Initialize(hInstance))
	{
		return FALSE;
	}

	_GameProcess->MessageLoop();

	delete _GameProcess;
	_GameProcess = nullptr;
	return 0;
}