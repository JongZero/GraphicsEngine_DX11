#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#pragma once

#include "Scene.h"


class YAMLParser;

/// <summary>
/// 씬들을 관리하는 매니저
/// 
/// 2021.11.03 강주현
/// </summary>
class SceneManager
{
public:
	//씬
	std::vector<Scene*> m_Scenes;		//현재 씬매니저에 속해있는 씬들
	
	Scene* m_NowScene;					//현재 씬

	int m_ClientWidth;					//현재 화면의 너비 

	int m_ClientHeight;					//현재 화면의 높이

	YAMLParser*	m_YamlParser;			//유니티 씬파일을 읽는 파서


public:
	//게임 오브젝트 이름 중복 체크
	std::wstring CheckOverlappedSceneName(std::wstring name);

	//씬의 이름 변경
	void SetSceneName(std::wstring name, Scene* scene);

	//현재씬의 이름 변경
	void SetSceneName(std::wstring name);

	//씬을 전환
	void ChangeScene(Scene* scene);
	void ChangeScene(std::wstring name);
	//씬을 생성
	void CreateScene(std::wstring name);
	//유니티 씬을 생성
	void CreateUnityScene(std::wstring name);

	//씬 추가
	void AddScene(Scene* scene);
	//씬 제거
	void RemoveScene(Scene* scene);
	//유니티 씬 파일을 읽어서 데이터를 보유
	void LoadSceneFile(std::wstring FileName);

	//유니티 씬 파일을 읽어서 씬을 생성
	void CreateSceneBySceneFile(std::wstring FileName);
public:
	SceneManager();
	SceneManager(std::wstring name);
	SceneManager(int width,int height);

	~SceneManager();
};

#endif