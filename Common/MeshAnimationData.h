#pragma once
#include <string>
#include <vector>
#include <windows.h>

#include "EMath.h"

struct Mesh;

/// <summary>
/// 메쉬(본) 1개의 애니메이션 데이터
/// </summary>
struct MeshAnimationData
{
	Mesh* m_Mesh = nullptr;						// 이 애니메이션 데이터의 메쉬
	Mesh* m_ParentMesh = nullptr;				// 부모 메쉬

	std::string m_NodeName;						// 애니메이터 초기화 시 한 번은 노드네임으로 메쉬를 연결시켜야함

	std::vector<int> m_PosKeyFrameVec;			// Pos 키프레임
	std::vector<int> m_RotKeyFrameVec;			// Rot 키프레임

	std::vector<EMath::Vector3> m_PosSampleVec;			// Pos Sample
	std::vector<EMath::Quaternion> m_RotSampleVec;		// Rot Sample

	UINT m_PosStartKeyFrame = 0;
	UINT m_PosNowKeyFrame = 0;
	UINT m_PosNextKeyFrame = 0;

	UINT m_RotStartKeyFrame = 0;
	UINT m_RotNowKeyFrame = 0;
	UINT m_RotNextKeyFrame = 0;

	int m_PosKeyFrameIndex = 0;
	int m_RotKeyFrameIndex = 0;

	EMath::Vector3 m_StartPosSample;
	EMath::Vector3 m_NowPosSample;
	EMath::Vector3 m_NextPosSample;

	EMath::Quaternion m_StartRotSample;
	EMath::Quaternion m_NowRotSample;
	EMath::Quaternion m_NextRotSample;

	float m_PosPer = 0;
	float m_RotPer = 0;

	EMath::Matrix m_TranslationTM = EMath::Matrix::Identity;
	EMath::Matrix m_RotationTM = EMath::Matrix::Identity;

	/// 새로운 방법
	std::vector<EMath::Matrix> m_AnimationTMVec;		// 인덱스가 키프레임이다.
};

/// <summary>
/// 모든 메쉬(본)의 애니메이션 데이터 -> 하나의 모션
/// </summary>
struct Motion
{
public:
	std::string m_MotionName;
	std::string m_ObjectNodeName;			// 파일의 첫번째 메쉬의 노드 네임 (이것으로 오브젝트를 구분함)
	std::vector<MeshAnimationData*> m_AnimationDataVec;

	float m_NowTime = 0.0f;
	float m_TicksPerFrame = 0.0f;

	size_t m_FirstKeyFrame = 0;			// 이 모션의 첫번째 프레임
	size_t m_NowKeyFrame = 0;			// 이 모션의 현재 프레임
	size_t m_LastKeyFrame = 0;			// 이 모션의 마지막 프레임

	size_t m_MaxParentCount = 0;		// 최대 부모가 몇 개까지 있는지

	std::vector<Mesh*> m_BoneVec;		// 본을 모아놓은 벡터
	Mesh* m_SkinnedMesh = nullptr;

	std::vector<EMath::Matrix> m_BoneOffsetTransformVec;	// 본들의 오프셋 트랜스폼 벡터
	std::vector<std::string> m_BoneOffsetName;
};
