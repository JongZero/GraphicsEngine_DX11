#pragma once
#include <string>
#include <vector>

#include "EMath.h"
#include "LightDefine.h"

/// <summary>
/// 오브젝트 1개를 렌더링하는데 필요한 데이터
/// 2021. 10. 29 정종영
/// </summary>
struct Shared_ObjectData
{
	wchar_t m_Name[256] = { 0, };					// 오브젝트의 이름 (이 이름에 맞는 메쉬 데이터들을 리소스매니저로부터 가져올 것임)

	bool m_IsSkinningObject = false;				// 스키닝 오브젝트인가? 스키닝 오브젝트라면 SkinnedMeshRenderer로 보내짐
	EMath::Matrix m_World;							// 오브젝트의 월드 행렬 (Transform 컴포넌트)

	wchar_t m_DiffuseMapName[256] = { 0, };			// 텍스쳐(디퓨즈맵)의 이름
	wchar_t m_NormalMapName[256] = { 0, };			// 노말맵의 이름
	EMath::Matrix m_TexTransform;					// 텍스쳐의 트랜스폼

	LegacyMaterialData m_LegacyMaterialData;

	bool m_IsCastShadow = false;		// 그림자를 드리우는가? (쉐도우 맵에 포함시킬 오브젝트인가?)
};

/// <summary>
/// 파티클 시스템 1개에 필요한 데이터
/// 2022. 03. 15 정종영
/// </summary>
struct Shared_ParticleSystemData
{
	bool m_IsActive = true;				// 활성화 중인지?
	bool m_IsReset = false;				// 리셋 해야하는지?

	wchar_t m_Name[256] = { 0, };
	size_t m_MaxParticles = 0;			// 최대 파티클 갯수

	EMath::FLOAT3 m_EmitPos;								// 방출할 위치, Transform의 포지션 넣어주자
	EMath::FLOAT3 m_EmitDir = EMath::FLOAT3(0, 1, 0);		// 어느 방향으로 방출할지
};

/// <summary>
/// 1프레임을 렌더링하는데 필요한 데이터
/// 2021. 10. 29 정종영
/// </summary>
struct Shared_RenderingData
{
	// Time
	float m_dTime;
	float m_TotalTime;

	/// Camera Data
	EMath::Vector3 m_CameraPosition;	// 현재 카메라의 월드 포지션
	EMath::Matrix m_View;				// 현재 카메라의 뷰 행렬
	EMath::Matrix m_Proj;				// 현재 카메라의 프로젝션 행렬

	float m_FarZ = 0;
	float m_NearZ = 0;

	/// Object
	std::vector<Shared_ObjectData*> m_ObjectDataForRenderVec;	// 오브젝트를 그리는데 필요한 데이터들

	/// Light
	std::vector<DirectionalLightInfo*> m_DirectionalLightInfoVec;
	std::vector<PointLightInfo*> m_PointLightInfoVec;
	std::vector<SpotLightInfo*> m_SpotLightInfoVec;

	/// Picking
	wchar_t m_PickedObject[256] = { 0, };		// 픽 된 오브젝트

	// Debug
	bool m_IsRenderTargetsShow = false;	// 디퍼드 렌더링에 쓰인 렌더타겟들을 보일 것인지
	bool m_IsFXAAOn = false;			// FXAA ON/OFF
	bool m_IsBloomOn = false;			// Bloom ON/OFF
};
