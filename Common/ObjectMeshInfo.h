#pragma once
#include <string>
#include <vector>

#include "EMath.h"
using namespace EMath;

/// <summary>
/// Mesh Filter에서 그래픽스 엔진의 메쉬 크리에이터에게 이런이런 메쉬를 만들어달라고 요청하는 견적서
/// 2021. 10. 27 정종영
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
		Now,	// 렌더 스테이트 클래스에서 현재 전역적으로 사용 중인 렌더 스테이트를 씀
	};

	enum class eEffectType
	{
		Color,
		Basic,
		NormalMap,
		Skinning,

		// 나중에 더 세분화 시켜야 될 수도 있다.
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
