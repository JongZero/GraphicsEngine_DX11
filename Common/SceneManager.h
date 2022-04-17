#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#pragma once

#include "Scene.h"


class YAMLParser;

/// <summary>
/// ������ �����ϴ� �Ŵ���
/// 
/// 2021.11.03 ������
/// </summary>
class SceneManager
{
public:
	//��
	std::vector<Scene*> m_Scenes;		//���� ���Ŵ����� �����ִ� ����
	
	Scene* m_NowScene;					//���� ��

	int m_ClientWidth;					//���� ȭ���� �ʺ� 

	int m_ClientHeight;					//���� ȭ���� ����

	YAMLParser*	m_YamlParser;			//����Ƽ �������� �д� �ļ�


public:
	//���� ������Ʈ �̸� �ߺ� üũ
	std::wstring CheckOverlappedSceneName(std::wstring name);

	//���� �̸� ����
	void SetSceneName(std::wstring name, Scene* scene);

	//������� �̸� ����
	void SetSceneName(std::wstring name);

	//���� ��ȯ
	void ChangeScene(Scene* scene);
	void ChangeScene(std::wstring name);
	//���� ����
	void CreateScene(std::wstring name);
	//����Ƽ ���� ����
	void CreateUnityScene(std::wstring name);

	//�� �߰�
	void AddScene(Scene* scene);
	//�� ����
	void RemoveScene(Scene* scene);
	//����Ƽ �� ������ �о �����͸� ����
	void LoadSceneFile(std::wstring FileName);

	//����Ƽ �� ������ �о ���� ����
	void CreateSceneBySceneFile(std::wstring FileName);
public:
	SceneManager();
	SceneManager(std::wstring name);
	SceneManager(int width,int height);

	~SceneManager();
};

#endif