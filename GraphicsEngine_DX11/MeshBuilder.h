#pragma once
#include "GeometryGenerator.h"
#include "ObjectMeshInfo.h"
#include "ObjectMeshData.h"
#include "ErrChecker.h"
#include "ParserData.h"
#include "DX11Core.h"

#include <memory>

class ResourceManager;
class GeometryGenerator;

/// <summary>
/// GeometryGenerator���� �����ϰų� ASE, FBX ������ �ε��� �о���� Vertex, Index ������ �� Ŭ�������� 
/// ���������� ���۷� �����. ���� �Ŀ��� ResourceManager�� �̸��� �Բ� �����ϰ�, �Ŀ� ���ӿ������κ��� �̸��� �޾ƿ�
/// ResourceManager ���ο� �̸��� ���Ͽ� �̸��� ������ �׸���.
/// 2021. 10. 27 ������
/// </summary>
class MeshBuilder
{
public:
	MeshBuilder(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm);
	~MeshBuilder();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_ResourceManager;
	std::unique_ptr<GeometryGenerator> m_GeometryGenerator;

private:
	// Picking�� BoundingBox�� ���� üũ ����
	ObjectMeshData* m_MyObjectMeshData;

public:
	// ������Ʈ���� ���� �����������
	void CreateMeshes(std::wstring objName, ObjectMeshInfo& objectMeshInfo);

private:
	// FBXParser�κ��� �����͸� ������ ������ ���
	void CreateMeshes_FromFBX(std::wstring fileName);

private:
	// ������Ʈ���� ���� �����������
	void CopyGeometryDataAndCreateBuffers(ObjectMeshData* pObjectMeshData, GeometryGenerator::GeoData& meshData, bool isPickingMesh = false);

	// FBXParser�κ��� �����͸� ���� ���
	void CopyGeometryDataAndCreateBuffers(ObjectMeshData* pObjectMeshData, std::vector<ParserData::Mesh*> fbxMeshVec);

	void LoadIndices(ObjectMeshData* pObjectMeshData, std::vector<ParserData::Mesh*> fbxMeshVec);

	void MakeTangent(std::vector<ParserData::Mesh*> meshDataVec);

	template<typename T> void CreateBuffers(ObjectMeshData* pObjectMeshData, std::vector<T>& vertices);

	// Picking�� ���� BoundingBox ����
	void CreateBoundingBoxForPicking(std::wstring objName, ObjectMeshData* pObjectMeshData);
};

template<typename T>
void MeshBuilder::CreateBuffers(ObjectMeshData* pObjectMeshData, std::vector<T>& vertices)
{
	pObjectMeshData->m_Stride = sizeof(T);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = pObjectMeshData->m_Stride * (UINT)pObjectMeshData->m_VertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_DX11Core->GetDevice()->CreateBuffer(&vbd, &vinitData, pObjectMeshData->m_VertexBuffer.GetAddressOf()));

	size_t size = pObjectMeshData->m_IndexVec.size();

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * (UINT)pObjectMeshData->m_IndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &pObjectMeshData->m_IndexVec[0];
	HR(m_DX11Core->GetDevice()->CreateBuffer(&ibd, &iinitData, pObjectMeshData->m_IndexBuffer.GetAddressOf()));
}