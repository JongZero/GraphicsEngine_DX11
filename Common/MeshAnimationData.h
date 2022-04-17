#pragma once
#include <string>
#include <vector>
#include <windows.h>

#include "EMath.h"

struct Mesh;

/// <summary>
/// �޽�(��) 1���� �ִϸ��̼� ������
/// </summary>
struct MeshAnimationData
{
	Mesh* m_Mesh = nullptr;						// �� �ִϸ��̼� �������� �޽�
	Mesh* m_ParentMesh = nullptr;				// �θ� �޽�

	std::string m_NodeName;						// �ִϸ����� �ʱ�ȭ �� �� ���� ���������� �޽��� ������Ѿ���

	std::vector<int> m_PosKeyFrameVec;			// Pos Ű������
	std::vector<int> m_RotKeyFrameVec;			// Rot Ű������

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

	/// ���ο� ���
	std::vector<EMath::Matrix> m_AnimationTMVec;		// �ε����� Ű�������̴�.
};

/// <summary>
/// ��� �޽�(��)�� �ִϸ��̼� ������ -> �ϳ��� ���
/// </summary>
struct Motion
{
public:
	std::string m_MotionName;
	std::string m_ObjectNodeName;			// ������ ù��° �޽��� ��� ���� (�̰����� ������Ʈ�� ������)
	std::vector<MeshAnimationData*> m_AnimationDataVec;

	float m_NowTime = 0.0f;
	float m_TicksPerFrame = 0.0f;

	size_t m_FirstKeyFrame = 0;			// �� ����� ù��° ������
	size_t m_NowKeyFrame = 0;			// �� ����� ���� ������
	size_t m_LastKeyFrame = 0;			// �� ����� ������ ������

	size_t m_MaxParentCount = 0;		// �ִ� �θ� �� ������ �ִ���

	std::vector<Mesh*> m_BoneVec;		// ���� ��Ƴ��� ����
	Mesh* m_SkinnedMesh = nullptr;

	std::vector<EMath::Matrix> m_BoneOffsetTransformVec;	// ������ ������ Ʈ������ ����
	std::vector<std::string> m_BoneOffsetName;
};
