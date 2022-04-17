#ifndef IGRAPHICSENGINE_H
#define IGRAPHICSENGINE_H
#include "GraphicsEngineMacroDefine.h"

/// <summary>
/// ���� ������ ����Ǵ� �׷��Ƚ� ���� Ŭ����
/// 2021. 12. 5 ������
/// </summary>
__interface IGraphicsEngine
{
public:
	virtual bool GetIsEndGraphicsEngineInitialization() abstract;

public:
	/// ���ҽ�
	virtual void LoadAllResourcesCount(std::wstring folderPath, ParserType type) abstract;
	virtual void LoadAllResources() abstract;

	/// â ũ�� ���� & ���ʱ�ȭ
	virtual void SetClientSize(int clientWidth, int clientHeight) abstract;
	virtual void OnResize() abstract;

	/// �޽�
	virtual void CreateMeshes(std::wstring objName, ObjectMeshInfo& objectMeshInfo) abstract;
	virtual void DeleteMeshes(std::wstring objName) abstract;

	/// �ִϸ��̼�
	virtual void InitializeAnimation(std::wstring objName, bool& isCompleted) abstract;
	virtual void UpdateAnimation(std::wstring objName, std::wstring nowMotionName, float dTime, float speed) abstract;

	/// ������
	virtual void StartRender(Shared_RenderingData* dataForRender) abstract;

	/// ��ŷ
	virtual void PickCheck(int x, int y) abstract;

	/// ��ƼŬ �ý���
	virtual void InitializeParticleSystem(Shared_ParticleSystemData* pData) abstract;
	virtual void ReleaseParticleSystem(std::wstring name) abstract;
};
#endif