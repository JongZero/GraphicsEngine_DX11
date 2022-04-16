#pragma once
#include "ObjectMeshInfo.h"
#include "ECollision.h"
#include <d3d11.h>
#include <wrl.h>
#include <memory>

struct Mesh;

/// <summary>
/// ������Ʈ 1���� �޽� ������, ���� �������κ��� ���ƿ� 
/// ������Ʈ�� �޽� ����(MeshType, EffectType, RenderStateType, �ʱ� scale, color...)�� ����
/// DX���۸� ����� 1���� ������Ʈ�� �޽��� �׸��µ� �ʿ��� ��� �����͵��� ���´�.
/// 2021. 10. 28 ������
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
	std::unique_ptr<ObjectMeshData> m_BoundingBoxForPickingMeshData;	// Ȯ���� ���� ����׿� �޽�
	std::vector<UINT> m_IndexVec;										// ��ŷ �� �޽� ��ȸ�� ���� �ε����� ������ ����
	std::vector<EMath::FLOAT3> m_VertexPosVec;

	std::vector<EMath::FLOAT4X4> m_BoneFinalTransformVec;				// ������ ���� Ʈ������
};