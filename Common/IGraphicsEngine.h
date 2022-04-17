#ifndef IGRAPHICSENGINE_H
#define IGRAPHICSENGINE_H
#include "GraphicsEngineMacroDefine.h"

/// <summary>
/// 게임 엔진에 노출되는 그래픽스 엔진 클래스
/// 2021. 12. 5 정종영
/// </summary>
__interface IGraphicsEngine
{
public:
	virtual bool GetIsEndGraphicsEngineInitialization() abstract;

public:
	/// 리소스
	virtual void LoadAllResourcesCount(std::wstring folderPath, ParserType type) abstract;
	virtual void LoadAllResources() abstract;

	/// 창 크기 설정 & 재초기화
	virtual void SetClientSize(int clientWidth, int clientHeight) abstract;
	virtual void OnResize() abstract;

	/// 메쉬
	virtual void CreateMeshes(std::wstring objName, ObjectMeshInfo& objectMeshInfo) abstract;
	virtual void DeleteMeshes(std::wstring objName) abstract;

	/// 애니메이션
	virtual void InitializeAnimation(std::wstring objName, bool& isCompleted) abstract;
	virtual void UpdateAnimation(std::wstring objName, std::wstring nowMotionName, float dTime, float speed) abstract;

	/// 렌더링
	virtual void StartRender(Shared_RenderingData* dataForRender) abstract;

	/// 피킹
	virtual void PickCheck(int x, int y) abstract;

	/// 파티클 시스템
	virtual void InitializeParticleSystem(Shared_ParticleSystemData* pData) abstract;
	virtual void ReleaseParticleSystem(std::wstring name) abstract;
};
#endif