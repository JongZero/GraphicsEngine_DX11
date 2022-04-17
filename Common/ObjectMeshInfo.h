#pragma once
#include <string>
#include <vector>

#include "EMath.h"
using namespace EMath;

/// <summary>
/// Mesh Filter���� �׷��Ƚ� ������ �޽� ũ�������Ϳ��� �̷��̷� �޽��� �����޶�� ��û�ϴ� ������
/// 2021. 10. 27 ������
/// </summary>
struct ObjectMeshInfo
{
public:
	enum class eMeshType
	{
		None,
		Box,
		ColorBox,
		Plane,
		Sphere,
		Geosphere,
		Cylinder,
		Skull,
		FBXERJ,
		Grid,
		Axis,
		//Frustum,
		FullScreenQuad,
		Cube,
	};

	enum class eRenderStateType
	{
		Solid,
		Wire,
		Now,	// ���� ������Ʈ Ŭ�������� ���� ���������� ��� ���� ���� ������Ʈ�� ��
	};

	enum class eEffectType
	{
		Color,
		Basic,
		NormalMap,
		Skinning,

		// ���߿� �� ����ȭ ���Ѿ� �� ���� �ִ�.
		// ex) GameObject	- Texture
		//					- Animation
		//					- Skinning
	};

public:
	ObjectMeshInfo(eMeshType mt, eRenderStateType rs, eEffectType et, 
		EMath::Vector4 color = EMath::Vector4(0, 1, 1, 1),
		EMath::Vector3 scale = EMath::Vector3(1, 1, 1), 
		EMath::Vector3 center = EMath::Vector3(0, 0, 0))
		: m_MeshType(mt), m_RenderStateType(rs), m_EffectType(et), m_Color(color), 
		m_Scale(scale), m_Center(center) {}

public:
	eMeshType m_MeshType = eMeshType::None;
	eRenderStateType m_RenderStateType = eRenderStateType::Now;
	eEffectType m_EffectType = eEffectType::Basic;

	EMath::Vector4 m_Color;
	EMath::Vector3 m_Scale;
	EMath::Vector3 m_Center;
};
