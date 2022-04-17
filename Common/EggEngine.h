#ifndef EGGENGINE_H
#define EGGENGINE_H
#pragma once
#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

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
/// ���� ����
/// 2021. 10. 25. ������
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
	// ���� ��ǲ�Ŵ��� �����͸� ����ְ�
	// �� ���� �ÿ� ��ǲ�Ŵ����� �ڵ����� �ٿ��ָ� ������ 
	// (������ ��ǲ�Ŵ����� �ٷ������ؼ� �� �� �ֵ���)
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
/// ����� ���� �Լ�
/// </summary>
public:
	//�������� ���� �� ����
	void CreateSceneBySceneFile(std::wstring sceneName);

	//���� ����
	void CreateScene(std::wstring sceneName);

	//������� �̸� ����
	void SetSceneName(std::wstring name);

	//������ ������Ʈ�� ����
	void CreateGameObject(std::wstring name);

	//������ ������Ʈ�� ����
	void RemoveGameObject(std::wstring name);

	//������ ������Ʈ�� ��ȯ
	GameObject* GetGameObject(std::wstring name);

	//�� �̸� ��ȯ
	std::wstring GetName() const;

	//�� ������ ���ӿ�����Ʈ�� �̸� ��ȯ
	std::wstring GetGameObjectName(int index);
	//�� ������ ���ӿ�����Ʈ ���� ��ȯ
	int GetGameObjectSize();
	//������Ʈ�� ��ŷ�Ѵ�.
	void PickCheck(int posx, int posy);
	//������Ʈ�� �̸����� ��ŷ�Ѵ�.
	void SetPickedObject(std::wstring name);

	//���� ���� �׽�Ʈ
	void CreateHeader(std::wstring name);
	void CreateCpp(std::wstring name);

	std::wstring SelectedName;
public:
	void Start();
	void Update();
	void Render();

public:
	// Timer ����
	void StopEngineTimer();
	void StartEngineTimer();
};

#endif
