#pragma once
#include "ObjectMeshInfo.h"
#include "ECollision.h"
#include <d3d11.h>
#include <wrl.h>
#include <memory>

struct Mesh;

/// <summary>
/// 오브젝트 1개의 메쉬 데이터, 게임 엔진으로부터 날아온 
/// 오브젝트의 메쉬 정보(MeshType, EffectType, RenderStateType, 초기 scale, color...)를 갖고
/// DX버퍼를 비롯한 1개의 오브젝트의 메쉬를 그리는데 필요한 모든 데이터들을 갖는다.
/// 2021. 10. 28 정종영
/// </summary>
struct ObjectMeshData
{
public:
	ObjectMeshData(ObjectMeshInfo omi)
		: m_ObjectMeshInfo(omi), m_VertexBuffer(nullptr), m_IndexBuffer(nullptr),
		m_VertexCount(0), m_IndexCount(0), m_Stride(0), m_BoundingBoxForPickingMeshData(nullptr) {}

public:
	/// Mesh Filter
	ObjectMeshInfo m_ObjectMeshInfo;

	/// DX11 Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;

	/// Meshes
	size_t m_VertexCount;
	size_t m_IndexCount;
	UINT m_Stride;
	std::vector<Mesh*> m_MeshVec;

	/// For Picking
	EMath::Vector3 m_MinVertexPos = EMath::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
	EMath::Vector3 m_MaxVertexPos = EMath::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);;
	ECollision::BoundingBox m_BoundingBoxForPicking;
	std::unique_ptr<ObjectMeshData> m_BoundingBoxForPickingMeshData;	// 확인을 위한 디버그용 메쉬
	std::vector<UINT> m_IndexVec;										// 피킹 시 메쉬 순회를 위해 인덱스와 포지션 저장
	std::vector<EMath::FLOAT3> m_VertexPosVec;

	std::vector<EMath::FLOAT4X4> m_BoneFinalTransformVec;				// 본들의 최종 트랜스폼
};