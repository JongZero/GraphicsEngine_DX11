#pragma once
#include <string>
#include <vector>

#include "EMath.h"
#include "LightDefine.h"

/// <summary>
/// ������Ʈ 1���� �������ϴµ� �ʿ��� ������
/// 2021. 10. 29 ������
/// </summary>
struct Shared_ObjectData
{
	wchar_t m_Name[256] = { 0, };					// ������Ʈ�� �̸� (�� �̸��� �´� �޽� �����͵��� ���ҽ��Ŵ����κ��� ������ ����)

	bool m_IsSkinningObject = false;				// ��Ű�� ������Ʈ�ΰ�? ��Ű�� ������Ʈ��� SkinnedMeshRenderer�� ������
	EMath::Matrix m_World;							// ������Ʈ�� ���� ��� (Transform ������Ʈ)

	wchar_t m_DiffuseMapName[256] = { 0, };			// �ؽ���(��ǻ���)�� �̸�
	wchar_t m_NormalMapName[256] = { 0, };			// �븻���� �̸�
	EMath::Matrix m_TexTransform;					// �ؽ����� Ʈ������

	LegacyMaterialData m_LegacyMaterialData;

	bool m_IsCastShadow = false;		// �׸��ڸ� �帮��°�? (������ �ʿ� ���Խ�ų ������Ʈ�ΰ�?)
};

/// <summary>
/// ��ƼŬ �ý��� 1���� �ʿ��� ������
/// 2022. 03. 15 ������
/// </summary>
struct Shared_ParticleSystemData
{
	bool m_IsActive = true;				// Ȱ��ȭ ������?
	bool m_IsReset = false;				// ���� �ؾ��ϴ���?

	wchar_t m_Name[256] = { 0, };
	size_t m_MaxParticles = 0;			// �ִ� ��ƼŬ ����

	EMath::FLOAT3 m_EmitPos;								// ������ ��ġ, Transform�� ������ �־�����
	EMath::FLOAT3 m_EmitDir = EMath::FLOAT3(0, 1, 0);		// ��� �������� ��������
};

/// <summary>
/// 1�������� �������ϴµ� �ʿ��� ������
/// 2021. 10. 29 ������
/// </summary>
struct Shared_RenderingData
{
	// Time
	float m_dTime;
	float m_TotalTime;

	/// Camera Data
	EMath::Vector3 m_CameraPosition;	// ���� ī�޶��� ���� ������
	EMath::Matrix m_View;				// ���� ī�޶��� �� ���
	EMath::Matrix m_Proj;				// ���� ī�޶��� �������� ���

	float m_FarZ = 0;
	float m_NearZ = 0;

	/// Object
	std::vector<Shared_ObjectData*> m_ObjectDataForRenderVec;	// ������Ʈ�� �׸��µ� �ʿ��� �����͵�

	/// Light
	std::vector<DirectionalLightInfo*> m_DirectionalLightInfoVec;
	std::vector<PointLightInfo*> m_PointLightInfoVec;
	std::vector<SpotLightInfo*> m_SpotLightInfoVec;

	/// Picking
	wchar_t m_PickedObject[256] = { 0, };		// �� �� ������Ʈ

	// Debug
	bool m_IsRenderTargetsShow = false;	// ���۵� �������� ���� ����Ÿ�ٵ��� ���� ������
	bool m_IsFXAAOn = false;			// FXAA ON/OFF
	bool m_IsBloomOn = false;			// Bloom ON/OFF
};
