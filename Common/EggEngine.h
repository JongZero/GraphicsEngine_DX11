#ifndef EGGENGINE_H
#define EGGENGINE_H
#pragma once
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.

#include <windows.h>
#include <string>
#include "SceneManager.h"
#include "Timer.h"

// Engine
#ifdef _WIN64
#ifdef _DEBUG
#pragma comment(lib, "EngineCore_x64_debug")
#else
#pragma comment(lib, "EngineCore_x64_release")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "EngineCore_x86_debug")
#else
#pragma comment(lib, "EngineCore_x86_release")
#endif
#endif

__interface IGraphicsEngine;
class Timer;
class SceneManager;
class GameObject;
class InputManager;

/// <summary>
/// 게임 엔진
/// 2021. 10. 25. 정종영
/// </summary>
class EggEngine
{
public:
	EggEngine(HWND hWnd, int clientWidth, int clientHeight);
	~EggEngine();

private:
	static Timer* m_pTimer;
	SceneManager* m_pSceneManager;
	InputManager* m_pInputManager;

public:
	// 씬이 인풋매니저 포인터를 들고있고
	// 씬 생성 시에 인풋매니저를 자동으로 붙여주면 좋을듯 
	// (씬에서 인풋매니저에 바로접근해서 쓸 수 있도록)
	InputManager* GetInputManager() { return m_pInputManager; }
	static float GetTotalTime() { return m_pTimer->TotalTime(); }

private:
	IGraphicsEngine* m_pGraphicsEngine;
	bool m_IsEndGraphicsEngineInitialization;

	float m_FPS;
	float m_FrameTime;

	GameObject* m_Player;

private:
	void CalculateFrameStats();

public:
	void SetClientSize(int clientWidth, int clientHeight);
	void OnResize();

public:
	bool GetIsEndGraphicsEngineInitialization();



/// <summary>
/// 사용자 편의 함수
/// </summary>
public:
	//씬파일을 통한 씬 생성
	void CreateSceneBySceneFile(std::wstring sceneName);

	//씬을 생성
	void CreateScene(std::wstring sceneName);

	//현재씬의 이름 변경
	void SetSceneName(std::wstring name);

	//씬에서 오브젝트를 생성
	void CreateGameObject(std::wstring name);

	//씬에서 오브젝트를 삭제
	void RemoveGameObject(std::wstring name);

	//씬에서 오브젝트를 반환
	GameObject* GetGameObject(std::wstring name);

	//씬 이름 반환
	std::wstring GetName() const;

	//씬 내부의 게임오브젝트의 이름 반환
	std::wstring GetGameObjectName(int index);
	//씬 내부의 게임오브젝트 개수 반환
	int GetGameObjectSize();
	//오브젝트를 피킹한다.
	void PickCheck(int posx, int posy);
	//오브젝트를 이름으로 피킹한다.
	void SetPickedObject(std::wstring name);

	//파일 생성 테스트
	void CreateHeader(std::wstring name);
	void CreateCpp(std::wstring name);

	std::wstring SelectedName;
public:
	void Start();
	void Update();
	void Render();

public:
	// Timer 관련
	void StopEngineTimer();
	void StartEngineTimer();
};

#endif
