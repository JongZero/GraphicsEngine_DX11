#include "framework.h"
#include "GameProcess.h"
#include "../Common/EggEngine.h"
#include "../Common/InputManager.h"
#include "../EngineCore/CParticleSystem.h"

namespace // static 안쓰고 MsgProc 세팅하기
{
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	GameProcess* g_GameProcess = 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return g_GameProcess->MsgProc(hWnd, msg, wParam, lParam);
}

GameProcess::GameProcess()
	:m_hWnd(nullptr),
	m_IsAppPaused(false), m_IsMinimized(false), m_IsMaximized(false), m_IsResizing(false),
	m_ClientWidth(1920), m_ClientHeight(1080), m_pInputManager(nullptr)
{
	g_GameProcess = this;
}

GameProcess::~GameProcess()
{
}

bool GameProcess::Initialize(HINSTANCE hInstance)
{
	/// 윈도를 등록한다.
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _T("EngineDemo");
	wcex.hIconSm = NULL;

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// 애플리케이션 초기화를 수행합니다:
	m_hWnd = CreateWindowW(_T("EngineDemo"), _T("EngineDemo"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, m_ClientWidth, m_ClientHeight, nullptr, nullptr, hInstance, nullptr);

	if (!m_hWnd)
	{
		MessageBox(0, L"Create Window Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hWnd, SW_SHOWNORMAL);
	UpdateWindow(m_hWnd);

	// 클라이언트 영역이 원하는 게임 해상도가 되도록 윈도 사이즈 재조정
	RecalcWindowSize();

	m_pEggEngine = std::make_unique<EggEngine>(m_hWnd, m_ClientWidth, m_ClientHeight);

	// Test 씬을 만든다.
	m_pEggEngine->CreateSceneBySceneFile(L"Test.unity");
	
	CParticleSystem* ps = new CParticleSystem;
	//
	m_pEggEngine->CreateGameObject(L"Fire");
	m_pEggEngine->GetGameObject(L"Fire")->AddComponent(ps);
	m_pEggEngine->GetGameObject(L"Fire")->GetComponent<Transform>()->SetPosition(EMath::Vector3(0, 1, 0));
   
	/// Unity씬 파일을 불러온다?
	m_pEggEngine->Start();

	m_pInputManager = m_pEggEngine->GetInputManager();

	return true;
}

void GameProcess::MessageLoop()
{
	MSG msg;

	// 기본 메시지 루프입니다
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			if (m_pInputManager->IsKeyReleased(InputManager::Keys::Escape))
			{
				break;
			}

			if (!m_IsAppPaused)
			{
				GameLoop();
			}
			else
			{
				Sleep(100);
			}
		}
	}
}

void GameProcess::GameLoop()
{
	m_pEggEngine->Update();

	m_pEggEngine->Render();
}

void GameProcess::RecalcWindowSize()
{
	RECT nowRect;
	DWORD _style = (DWORD)GetWindowLong(m_hWnd, GWL_STYLE);
	DWORD _exstyle = (DWORD)GetWindowLong(m_hWnd, GWL_EXSTYLE);

	GetWindowRect(m_hWnd, &nowRect);

	RECT newRect;
	newRect.left = 0;
	newRect.top = 0;
	newRect.right = m_ClientWidth;
	newRect.bottom = m_ClientHeight;

	AdjustWindowRectEx(&newRect, _style, NULL, _exstyle);

	// 클라이언트 영역보다 윈도 크기는 더 커야 한다. (외곽선, 타이틀 등)
	int _newWidth = (newRect.right - newRect.left);
	int _newHeight = (newRect.bottom - newRect.top);

	SetWindowPos(m_hWnd, HWND_NOTOPMOST, nowRect.left, nowRect.top,
		_newWidth, _newHeight, SWP_SHOWWINDOW);
}

LRESULT CALLBACK GameProcess::MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	// We pause the game when the window is deactivated and unpause it 
	// when it becomes active.  
	case WM_ACTIVATE:
		if (m_pEggEngine != nullptr)
		{
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				m_IsAppPaused = true;
				m_pEggEngine->StopEngineTimer();
			}
			else
			{
				m_IsAppPaused = false;
				m_pEggEngine->StartEngineTimer();
			}
		}
		return 0;

	// 창의 최소크기 설정
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;
	case WM_ACTIVATEAPP:
		if (m_pInputManager != nullptr)
		{
			m_pInputManager->KeyboardProcessMessage(message, wParam, lParam);
		}
		return 0;
	case WM_KEYDOWN:
		if (m_pInputManager != nullptr)
		{
			m_pInputManager->KeyboardProcessMessage(message, wParam, lParam);
		}
		return 0;
	case WM_SYSKEYDOWN:
		if (m_pInputManager != nullptr)
		{
			m_pInputManager->KeyboardProcessMessage(message, wParam, lParam);
		}
		return 0;
	case WM_KEYUP:
		if (m_pInputManager != nullptr)
		{
			m_pInputManager->KeyboardProcessMessage(message, wParam, lParam);
		}
		return 0;
	case WM_SYSKEYUP:
		if (m_pInputManager != nullptr)
		{
			m_pInputManager->KeyboardProcessMessage(message, wParam, lParam);
		}
		return 0;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if (m_pInputManager)
		{
			m_pInputManager->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			m_pInputManager->MouseProcessMessage(message, wParam, lParam);
		}
		return 0;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		if (m_pInputManager)
		{
			m_pInputManager->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			m_pInputManager->MouseProcessMessage(message, wParam, lParam);
		}
		return 0;
	case WM_MOUSEMOVE:
		if (m_pInputManager)
		{
			m_pInputManager->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if ((wParam & MK_RBUTTON) != 0)
			{
				POINT _deltaMousePos = m_pInputManager->GetDeltaMousePos();

				float _dx = EMath::ConvertToRadians(0.25f * m_pInputManager->GetDeltaMousePos().x);
				float _dy = EMath::ConvertToRadians(0.25f * m_pInputManager->GetDeltaMousePos().y);

				m_pEggEngine->GetGameObject(L"MainCamera")->GetComponent<Camera>()->Pitch(_dy);
				m_pEggEngine->GetGameObject(L"MainCamera")->GetComponent<Camera>()->RotateY(_dx);
			}

			m_pInputManager->MouseProcessMessage(message, wParam, lParam);
		}
		return 0;
	case WM_SIZE:
		// Save the new client area dimensions.
		m_ClientWidth = LOWORD(lParam);
		m_ClientHeight = HIWORD(lParam);

		if (m_pEggEngine != nullptr && m_pEggEngine->GetIsEndGraphicsEngineInitialization())
		{
			m_pEggEngine->SetClientSize(m_ClientWidth, m_ClientHeight);

			if (wParam == SIZE_MINIMIZED)
			{
				m_IsAppPaused = true;
				m_IsMinimized = true;
				m_IsMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_IsAppPaused = false;
				m_IsMinimized = false;
				m_IsMaximized = true;
				m_pEggEngine->OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{
				// Restoring from minimized state?
				if (m_IsMinimized)
				{
					m_IsAppPaused = false;
					m_IsMinimized = false;
					m_pEggEngine->OnResize();
				}

				// Restoring from maximized state?
				else if (m_IsMaximized)
				{
					m_IsAppPaused = false;
					m_IsMaximized = false;
					m_pEggEngine->OnResize();
				}
				else if (m_IsResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					m_pEggEngine->OnResize();
				}
			}
		}
		return 0;

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_IsAppPaused = true;
		m_IsResizing = true;
		m_pEggEngine->StopEngineTimer();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_IsAppPaused = false;
		m_IsResizing = false;
		m_pEggEngine->StartEngineTimer();
		m_pEggEngine->OnResize();
		return 0;

	// The WM_MENUCHAR message is sent when a menu is active and the user presses 
	// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}